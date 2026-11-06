#include <QApplication>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <openssl/evp.h>
#include <openssl/pem.h>

static bool signEd25519(const QByteArray &data, const QString &keyPath, QByteArray *signature, QString *error)
{
    QFile file(keyPath);
    if (!file.open(QIODevice::ReadOnly)) {
        *error = QStringLiteral("Не удалось открыть приватный ключ: %1").arg(keyPath);
        return false;
    }
    const QByteArray pem = file.readAll();
    BIO *bio = BIO_new_mem_buf(pem.constData(), pem.size());
    EVP_PKEY *key = bio ? PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr) : nullptr;
    if (bio) BIO_free(bio);
    if (!key) {
        *error = QStringLiteral("Файл не является корректным приватным ключом PEM.");
        return false;
    }
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    size_t required = 0;
    bool ok = ctx && EVP_DigestSignInit(ctx, nullptr, nullptr, nullptr, key) == 1
        && EVP_DigestSign(ctx, nullptr, &required,
                          reinterpret_cast<const unsigned char *>(data.constData()), size_t(data.size())) == 1;
    QByteArray sig;
    if (ok) {
        sig.resize(int(required));
        ok = EVP_DigestSign(ctx, reinterpret_cast<unsigned char *>(sig.data()), &required,
                            reinterpret_cast<const unsigned char *>(data.constData()), size_t(data.size())) == 1;
        sig.resize(int(required));
    }
    if (ctx) EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(key);
    if (!ok) {
        *error = QStringLiteral("Не удалось подписать лицензию. Проверьте приватный ключ.");
        return false;
    }
    *signature = sig;
    return true;
}

class Generator final : public QWidget
{
public:
    Generator()
    {
        setWindowTitle(QStringLiteral("PSVMS — Генератор лицензий"));
        setMinimumSize(620, 560);
        auto *root = new QVBoxLayout(this);
        auto *heading = new QLabel(QStringLiteral("Генератор лицензий PSVMS"), this);
        heading->setStyleSheet(QStringLiteral("font-size:24px;font-weight:800;color:#d8b64c;"));
        root->addWidget(heading);
        auto *description = new QLabel(QStringLiteral("Выпускайте подписанные лицензии Pro, привязанные к конкретному компьютеру."), this);
        description->setWordWrap(true);
        root->addWidget(description);

        auto *form = new QFormLayout;
        m_customer = new QLineEdit(this);
        m_customer->setPlaceholderText(QStringLiteral("Например: ООО Камера-Сервис"));
        m_machine = new QLineEdit(this);
        m_machine->setPlaceholderText(QStringLiteral("Вставьте ID из Настройки → Лицензия в PSVMS"));
        m_preset = new QComboBox(this);
        m_preset->addItem(QStringLiteral("Произвольный срок"), 0);
        m_preset->addItem(QStringLiteral("30 дней"), 30);
        m_preset->addItem(QStringLiteral("60 дней"), 60);
        m_preset->addItem(QStringLiteral("90 дней"), 90);
        m_preset->addItem(QStringLiteral("120 дней"), 120);
        m_days = new QSpinBox(this); m_days->setRange(0, 36500); m_days->setValue(30); m_days->setSuffix(QStringLiteral(" дн."));
        m_hours = new QSpinBox(this); m_hours->setRange(0, 23); m_hours->setSuffix(QStringLiteral(" ч."));
        m_minutes = new QSpinBox(this); m_minutes->setRange(0, 59); m_minutes->setSuffix(QStringLiteral(" мин."));
        auto *durationRow = new QWidget(this);
        auto *durationLayout = new QHBoxLayout(durationRow); durationLayout->setContentsMargins(0,0,0,0);
        durationLayout->addWidget(m_days); durationLayout->addWidget(m_hours); durationLayout->addWidget(m_minutes);
        m_expiry = new QDateTimeEdit(QDateTime::currentDateTime().addDays(30), this);
        m_expiry->setCalendarPopup(true);
        m_expiry->setDisplayFormat(QStringLiteral("dd.MM.yyyy HH:mm:ss"));
        m_mode = new QComboBox(this);
        m_mode->addItem(QStringLiteral("Срок от первой активации (дни/часы/минуты)"), QStringLiteral("duration"));
        m_mode->addItem(QStringLiteral("Точная дата и время окончания"), QStringLiteral("fixed"));
        m_keyPath = new QLineEdit(this);
        const QString defaultKey = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../../../keys/psvms_license_private.pem"));
        if (QFileInfo::exists(defaultKey)) m_keyPath->setText(QFileInfo(defaultKey).canonicalFilePath());
        auto *keyRow = new QWidget(this);
        auto *keyLayout = new QHBoxLayout(keyRow); keyLayout->setContentsMargins(0,0,0,0);
        keyLayout->addWidget(m_keyPath, 1);
        auto *browseKey = new QPushButton(QStringLiteral("Обзор…"), this);
        keyLayout->addWidget(browseKey);
        m_output = new QLineEdit(QDir::home().filePath(QStringLiteral("PSVMS_license.lic")), this);
        auto *outRow = new QWidget(this);
        auto *outLayout = new QHBoxLayout(outRow); outLayout->setContentsMargins(0,0,0,0);
        outLayout->addWidget(m_output, 1);
        auto *browseOut = new QPushButton(QStringLiteral("Сохранить как…"), this);
        outLayout->addWidget(browseOut);

        form->addRow(QStringLiteral("Клиент / организация:"), m_customer);
        form->addRow(QStringLiteral("ID компьютера:"), m_machine);
        form->addRow(QStringLiteral("Тип срока:"), m_mode);
        form->addRow(QStringLiteral("Готовый вариант:"), m_preset);
        form->addRow(QStringLiteral("Продолжительность:"), durationRow);
        form->addRow(QStringLiteral("Дата окончания:"), m_expiry);
        form->addRow(QStringLiteral("Приватный ключ:"), keyRow);
        form->addRow(QStringLiteral("Файл лицензии:"), outRow);
        auto *group = new QGroupBox(QStringLiteral("Параметры лицензии"), this);
        group->setLayout(form);
        root->addWidget(group);

        m_preview = new QLabel(this);
        m_preview->setWordWrap(true);
        m_preview->setStyleSheet(QStringLiteral("padding:10px;background:#202733;border-radius:8px;"));
        root->addWidget(m_preview);
        auto *buttons = new QHBoxLayout;
        auto *generate = new QPushButton(QStringLiteral("Создать подписанную лицензию"), this);
        generate->setStyleSheet(QStringLiteral("font-weight:700;padding:10px;"));
        auto *close = new QPushButton(QStringLiteral("Закрыть"), this);
        buttons->addWidget(generate); buttons->addWidget(close);
        root->addLayout(buttons);

        connect(browseKey, &QPushButton::clicked, this, [this] {
            const QString path = QFileDialog::getOpenFileName(this, QStringLiteral("Выберите приватный ключ"), QString(), QStringLiteral("PEM key (*.pem);;All files (*.*)"));
            if (!path.isEmpty()) m_keyPath->setText(path);
        });
        connect(browseOut, &QPushButton::clicked, this, [this] {
            const QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Сохранить лицензию"), m_output->text(), QStringLiteral("Лицензия PSVMS (*.lic)"));
            if (!path.isEmpty()) m_output->setText(path.endsWith(QStringLiteral(".lic"), Qt::CaseInsensitive) ? path : path + QStringLiteral(".lic"));
        });
        connect(m_preset, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int) {
            const int days = m_preset->currentData().toInt();
            if (days > 0) { m_days->setValue(days); m_hours->setValue(0); m_minutes->setValue(0); }
            updatePreview();
        });
        connect(m_mode, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { updateModeUi(); updatePreview(); });
        for (QSpinBox *spin : {m_days, m_hours, m_minutes}) connect(spin, qOverload<int>(&QSpinBox::valueChanged), this, [this] { m_preset->setCurrentIndex(0); updatePreview(); });
        connect(m_expiry, &QDateTimeEdit::dateTimeChanged, this, [this] { updatePreview(); });
        connect(generate, &QPushButton::clicked, this, [this] { createLicense(); });
        connect(close, &QPushButton::clicked, this, &QWidget::close);
        updateModeUi(); updatePreview();
    }

private:
    void updateModeUi()
    {
        const bool duration = m_mode->currentData().toString() == QStringLiteral("duration");
        m_preset->setEnabled(duration); m_days->setEnabled(duration); m_hours->setEnabled(duration); m_minutes->setEnabled(duration);
        m_expiry->setEnabled(!duration);
    }
    qint64 durationSeconds() const
    {
        return qint64(m_days->value()) * 86400 + qint64(m_hours->value()) * 3600 + qint64(m_minutes->value()) * 60;
    }
    void updatePreview()
    {
        if (m_mode->currentData().toString() == QStringLiteral("duration")) {
            const QDateTime estimated = QDateTime::currentDateTime().addSecs(durationSeconds());
            m_preview->setText(QStringLiteral("Срок: %1 дн. %2 ч. %3 мин.\nОкончание при активации сейчас: %4")
                .arg(m_days->value()).arg(m_hours->value()).arg(m_minutes->value())
                .arg(estimated.toString(QStringLiteral("dd.MM.yyyy HH:mm:ss"))));
        } else {
            m_preview->setText(QStringLiteral("Лицензия будет действовать до: %1")
                .arg(m_expiry->dateTime().toString(QStringLiteral("dd.MM.yyyy HH:mm:ss"))));
        }
    }
    void createLicense()
    {
        const QString machine = m_machine->text().trimmed().toUpper();
        if (m_customer->text().trimmed().isEmpty() || machine.size() != 32 || !QRegularExpression(QStringLiteral("^[0-9A-F]{32}$")).match(machine).hasMatch()) {
            QMessageBox::warning(this, QStringLiteral("Проверьте данные"), QStringLiteral("Укажите клиента и корректный 32-символьный ID компьютера из PSVMS.")); return;
        }
        if (m_keyPath->text().trimmed().isEmpty() || !QFileInfo::exists(m_keyPath->text().trimmed())) {
            QMessageBox::warning(this, QStringLiteral("Нет ключа"), QStringLiteral("Выберите приватный ключ psvms_license_private.pem.")); return;
        }
        if (m_output->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("Нет файла"), QStringLiteral("Укажите путь сохранения файла лицензии.")); return;
        }
        const QString mode = m_mode->currentData().toString();
        if (mode == QStringLiteral("duration") && durationSeconds() <= 0) {
            QMessageBox::warning(this, QStringLiteral("Неверный срок"), QStringLiteral("Продолжительность должна быть больше нуля.")); return;
        }
        if (mode == QStringLiteral("fixed") && m_expiry->dateTime().toUTC() <= QDateTime::currentDateTimeUtc()) {
            QMessageBox::warning(this, QStringLiteral("Неверная дата"), QStringLiteral("Дата окончания должна быть в будущем.")); return;
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("schema"), 1);
        payload.insert(QStringLiteral("licenseId"), QUuid::createUuid().toString(QUuid::WithoutBraces));
        payload.insert(QStringLiteral("customer"), m_customer->text().trimmed());
        payload.insert(QStringLiteral("machineId"), machine);
        payload.insert(QStringLiteral("mode"), mode);
        payload.insert(QStringLiteral("createdAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
        if (mode == QStringLiteral("duration")) payload.insert(QStringLiteral("durationSeconds"), double(durationSeconds()));
        else payload.insert(QStringLiteral("expiresAt"), m_expiry->dateTime().toUTC().toString(Qt::ISODateWithMs));

        const QByteArray data = QJsonDocument(payload).toJson(QJsonDocument::Compact);
        QByteArray signature; QString error;
        if (!signEd25519(data, m_keyPath->text().trimmed(), &signature, &error)) {
            QMessageBox::critical(this, QStringLiteral("Ошибка подписи"), error); return;
        }
        const QJsonObject root{{QStringLiteral("payload"), payload}, {QStringLiteral("signature"), QString::fromLatin1(signature.toBase64())}};
        QFile file(m_output->text().trimmed());
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QMessageBox::critical(this, QStringLiteral("Ошибка сохранения"), QStringLiteral("Не удалось записать файл лицензии.")); return;
        }
        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        file.close();
        QMessageBox::information(this, QStringLiteral("Готово"), QStringLiteral("Подписанная лицензия создана:\n%1\n\nПередайте файл клиенту. Приватный ключ никому не отправляйте.").arg(QFileInfo(file).absoluteFilePath()));
    }

    QLineEdit *m_customer = nullptr, *m_machine = nullptr, *m_keyPath = nullptr, *m_output = nullptr;
    QComboBox *m_preset = nullptr, *m_mode = nullptr;
    QSpinBox *m_days = nullptr, *m_hours = nullptr, *m_minutes = nullptr;
    QDateTimeEdit *m_expiry = nullptr;
    QLabel *m_preview = nullptr;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("PSVMS License Generator"));
    app.setStyle(QStringLiteral("Fusion"));
    Generator window;
    window.show();
    return app.exec();
}
