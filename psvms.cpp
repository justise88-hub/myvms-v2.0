// PSVMS — все исходники программы в одном файле.
// Собирается вместе с CMakeLists.txt из этой же папки (нужны Qt 6.8+, FFmpeg и OpenSSL).
// Файл собран автоматически из проекта; секции помечены исходными именами файлов.

// ======================================================================
// ВСЁ В ОДНОМ ФАЙЛЕ: ниже, в комментариях, — инструкция и файлы сборки.
// Чтобы собрать программу, создайте рядом с этим файлом файлы CMakeLists.txt, build_windows.bat
// и .github/workflows/build-windows.yml и скопируйте в них текст из разделов ниже (без «//   »).
// ======================================================================
//
//   --- README.md ---
//   # PSVMS — видеонаблюдение для Windows (проект в одном файле)
//
//   Весь код программы лежит в одном файле `psvms.cpp`. Готового `PSVMS.exe` в архиве нет:
//   его собирает GitHub (бесплатно), компьютер с Visual Studio для этого не нужен.
//
//   ## Как получить PSVMS.exe
//
//   1. Зайдите на github.com, создайте аккаунт и новый репозиторий (New repository).
//   2. Распакуйте этот архив и перетащите мышью **все файлы и папку `.github`** на страницу репозитория
//      (ссылка «uploading an existing file»), затем нажмите **Commit changes**.
//      Если папка `.github` не загрузилась: Add file → Create new file, введите имя
//      `.github/workflows/build-windows.yml` и вставьте текст из раздела «Файл сборки» ниже.
//   3. Откройте вкладку **Actions**, выберите «Build PSVMS for Windows» и нажмите **Run workflow**.
//      Первая сборка идёт 20-40 минут (собирается FFmpeg).
//   4. Когда сборка закончится, откройте её страницу и внизу, в разделе **Artifacts**, скачайте `PSVMS-windows`.
//      Распакуйте архив правой кнопкой («Извлечь всё») и запустите `PSVMS.exe` из этой папки.
//
//   Если сборка завершилась красным крестиком, откройте шаг **Build**, скопируйте строки со словом `error`
//   и отправьте их разработчику: код ещё ни разу не компилировался.
//
//   ## Первый запуск
//
//   - Пароль при первом запуске не нужен: программа входит автоматически под пользователем «Администратор».
//     Пароли и новые пользователи добавляются в «Настройки → Пользователи».
//   - Камеры: «Устройства → Добавить» (вручную, Hikvision / HiWatch по IP, поиск в сети, ONVIF, iCSee).
//   - Язык (русский или английский) выбирается в боковом меню.
//
//   ## Если у вас есть Windows с Visual Studio 2022, Qt 6.5+ и vcpkg
//
//   Поправьте два пути в начале `build_windows.bat` и запустите его: готовая программа окажется в `build\Release`.
//
//   ## Файл сборки (`.github/workflows/build-windows.yml`)
//
//   ```yaml
//   name: Build PSVMS for Windows
//
//   # Запуск: вкладка Actions -> "Build PSVMS for Windows" -> Run workflow.
//   # Готовая программа появится внизу страницы запуска в разделе Artifacts (PSVMS-windows.zip).
//   on:
//     workflow_dispatch:
//     push:
//       branches: [main, master]
//
//   jobs:
//     build:
//       runs-on: windows-2022
//       steps:
//         - uses: actions/checkout@v4
//
//         - name: Install Qt
//           uses: jurplel/install-qt-action@v4
//           with:
//             version: '6.8.1'
//             arch: 'win64_msvc2022_64'
//             modules: 'qtmultimedia'
//             cache: true
//
//         # FFmpeg собирается из исходников: первый запуск занимает 20-40 минут
//         - name: Install FFmpeg and OpenSSL (vcpkg)
//           shell: pwsh
//           run: |
//             & "$env:VCPKG_INSTALLATION_ROOT\vcpkg.exe" install "ffmpeg[core,avcodec,avformat]:x64-windows" openssl:x64-windows
//
//         - name: Configure
//           shell: pwsh
//           run: |
//             cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
//               -DCMAKE_PREFIX_PATH="$env:QT_ROOT_DIR" `
//               -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_INSTALLATION_ROOT\scripts\buildsystems\vcpkg.cmake"
//
//         - name: Build
//           run: cmake --build build --config Release
//
//         - name: Collect Qt libraries next to the exe
//           shell: pwsh
//           run: |
//             & "$env:QT_ROOT_DIR\bin\windeployqt.exe" --release --no-translations build\Release\PSVMS.exe
//
//         - name: Upload
//           uses: actions/upload-artifact@v4
//           with:
//             name: PSVMS-windows
//             path: build/Release/
//   ```
//
//   --- CMakeLists.txt ---
//   cmake_minimum_required(VERSION 3.21)
//   project(PSVMS LANGUAGES CXX)
//
//   set(CMAKE_CXX_STANDARD 17)
//   set(CMAKE_CXX_STANDARD_REQUIRED ON)
//   set(CMAKE_AUTOMOC ON)
//
//   if(MSVC)
//       add_compile_options(/utf-8)   # русские строки в исходниках
//   endif()
//
//   find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia MultimediaWidgets Sql Network)
//
//   # ---- FFmpeg: сначала pkg-config (Linux/macOS), иначе поиск файлов (Windows + vcpkg) ----
//   find_package(PkgConfig QUIET)
//   if(PkgConfig_FOUND)
//       pkg_check_modules(FFMPEG IMPORTED_TARGET libavformat libavcodec libavutil)
//   endif()
//
//   if(FFMPEG_FOUND)
//       set(FFMPEG_TARGET PkgConfig::FFMPEG)
//   else()
//       add_library(ffmpeg_deps INTERFACE)
//       foreach(lib avformat avcodec avutil)
//           find_path(FF_${lib}_INC lib${lib}/${lib}.h)
//           find_library(FF_${lib}_LIB ${lib})
//           if(NOT FF_${lib}_INC OR NOT FF_${lib}_LIB)
//               message(FATAL_ERROR "FFmpeg (${lib}) не найден. Установите FFmpeg (dev-пакеты) "
//                                   "или передайте -DCMAKE_PREFIX_PATH / vcpkg toolchain.")
//           endif()
//           target_include_directories(ffmpeg_deps INTERFACE ${FF_${lib}_INC})
//           target_link_libraries(ffmpeg_deps INTERFACE ${FF_${lib}_LIB})
//       endforeach()
//       set(FFMPEG_TARGET ffmpeg_deps)
//   endif()
//
//   # ---- Всё приложение — один файл исходников ----
//   qt_add_executable(PSVMS WIN32 psvms.cpp)
//   find_package(OpenSSL REQUIRED)
//   target_link_libraries(PSVMS PRIVATE
//       Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets Qt6::Sql Qt6::Network
//       OpenSSL::Crypto ${FFMPEG_TARGET})
//   if(WIN32)
//       target_link_libraries(PSVMS PRIVATE dwmapi)   # тёмная рамка окон в Windows 10/11
//   endif()
//
//   --- build_windows.bat ---
//   @echo off
//   REM ============================================================
//   REM  Build PSVMS.exe on Windows
//   REM  Requirements:
//   REM    1) Visual Studio 2022 with "Desktop development with C++"
//   REM    2) Qt 6.5+ for MSVC 2022 64-bit (Qt Multimedia + Qt SQL)
//   REM    3) vcpkg (https://github.com/microsoft/vcpkg)
//   REM  Edit the two paths below, then run this file.
//   REM ============================================================
//   set QT_DIR=C:\Qt\6.8.1\msvc2022_64
//   set VCPKG_ROOT=C:\vcpkg
//
//   if not exist "%QT_DIR%\bin\windeployqt.exe" (
//       echo Qt not found in %QT_DIR%
//       exit /b 1
//   )
//   if not exist "%VCPKG_ROOT%\vcpkg.exe" (
//       echo vcpkg not found in %VCPKG_ROOT%
//       exit /b 1
//   )
//
//   echo [1/4] Installing FFmpeg via vcpkg...
//   "%VCPKG_ROOT%\vcpkg.exe" install "ffmpeg[core,avcodec,avformat]:x64-windows" openssl:x64-windows || exit /b 1
//
//   echo [2/4] Configuring...
//   cmake -S . -B build -G "Visual Studio 17 2022" -A x64 ^
//     -DCMAKE_PREFIX_PATH="%QT_DIR%" ^
//     -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" || exit /b 1
//
//   echo [3/4] Building...
//   cmake --build build --config Release || exit /b 1
//
//   echo [4/4] Collecting Qt libraries next to the exe...
//   "%QT_DIR%\bin\windeployqt.exe" --release --no-translations build\Release\PSVMS.exe || exit /b 1
//
//   echo.
//   echo Done. Ready-to-run folder: build\Release   (PSVMS.exe)
//
//   --- .github/workflows/build-windows.yml ---
//   name: Build PSVMS for Windows
//
//   # Запуск: вкладка Actions -> "Build PSVMS for Windows" -> Run workflow.
//   # Готовая программа появится внизу страницы запуска в разделе Artifacts (PSVMS-windows.zip).
//   on:
//     workflow_dispatch:
//     push:
//       branches: [main, master]
//
//   jobs:
//     build:
//       runs-on: windows-2022
//       steps:
//         - uses: actions/checkout@v4
//
//         - name: Install Qt
//           uses: jurplel/install-qt-action@v4
//           with:
//             version: '6.8.1'
//             arch: 'win64_msvc2022_64'
//             modules: 'qtmultimedia'
//             cache: true
//
//         # FFmpeg собирается из исходников: первый запуск занимает 20-40 минут
//         - name: Install FFmpeg and OpenSSL (vcpkg)
//           shell: pwsh
//           run: |
//             & "$env:VCPKG_INSTALLATION_ROOT\vcpkg.exe" install "ffmpeg[core,avcodec,avformat]:x64-windows" openssl:x64-windows
//
//         - name: Configure
//           shell: pwsh
//           run: |
//             cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
//               -DCMAKE_PREFIX_PATH="$env:QT_ROOT_DIR" `
//               -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_INSTALLATION_ROOT\scripts\buildsystems\vcpkg.cmake"
//
//         - name: Build
//           run: cmake --build build --config Release
//
//         - name: Collect Qt libraries next to the exe
//           shell: pwsh
//           run: |
//             & "$env:QT_ROOT_DIR\bin\windeployqt.exe" --release --no-translations build\Release\PSVMS.exe
//
//         - name: Upload
//           uses: actions/upload-artifact@v4
//           with:
//             name: PSVMS-windows
//             path: build/Release/
//
// ======================================================================
// КОНЕЦ ВСТРОЕННЫХ ФАЙЛОВ. Дальше — код программы.
// ======================================================================

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSysInfo>
#include <QDateTime>
#include <QFile>
#include <QClipboard>
#include <openssl/evp.h>
#include <QThread>
#include <atomic>
#include <QHash>
#include <QList>
#include <QObject>
#include <QStringList>
#include <QDialog>
#include <QCoreApplication>
#include <QMimeData>
#include <QTreeWidget>
#include <QWidget>
#include <QUrl>
#include <QSet>
#include <QPointer>
#include <QMainWindow>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QFrame>
#include <QImage>
#include <QPixmap>
#include <QPoint>
#include <QRectF>
#include <QtGlobal>
#include <QFormLayout>
#include <QLabel>
#include <QMediaDevices>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QSlider>
#include <QDir>
#include <QSettings>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QTimer>
#include <QUuid>
#include <memory>
#include <QBrush>
#include <QColor>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenu>
#include <QRegularExpression>
#include <QTableWidget>
#include <QToolButton>
#include <QDateTime>
#include <QAudioOutput>
#include <QKeyEvent>
#include <QMediaPlayer>
#include <QScreen>
#include <QHostAddress>
#include <QProgressBar>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QGroupBox>
#include <QActionGroup>
#include <QEasingCurve>
#include <QGridLayout>
#include <QIcon>
#include <QMap>
#include <QPainter>
#include <QPolygonF>
#include <QPaintEvent>
#include <QPropertyAnimation>
#include <QSplitter>
#include <QTreeWidgetItem>
#include <QCloseEvent>
#include <QCursor>
#include <QKeySequence>
#include <QShortcut>
#include <QStackedWidget>
#include <QStatusBar>
#include <QCalendarWidget>
#include <QFileInfo>
#include <QListWidget>
#include <QVideoWidget>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTextEdit>
#include <QUrl>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QFileDialog>
#include <QSaveFile>
#include <QProcess>
#include <QTabWidget>
#include <functional>
#include <QButtonGroup>
#include <QEvent>
#include <QStyle>
#include <QApplication>
#include <QFontDatabase>
#include <QEventLoop>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QConicalGradient>
#include <QPainterPath>
#include <QSplashScreen>
#include <QCamera>
#include <QContextMenuEvent>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QEnterEvent>
#include <QGraphicsOpacityEffect>
#include <QMediaCaptureSession>
#include <QMouseEvent>
#include <QStackedLayout>
#include <QAction>
#include <QVideoFrame>
#include <QVideoSink>
#include <QWheelEvent>
#include <cmath>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>
#include <QScopeGuard>
#include <cerrno>
#include <cstdint>
#include <vector>
#include <QAuthenticator>
#include <QNetworkInterface>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QUdpSocket>
#include <QXmlStreamReader>
#include <QLocale>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <algorithm>
#include <QSqlDatabase>
#include <QSqlError>
static void writePsvmsLog(const QString &message);
#include <QMutex>
#include <QMutexLocker>
#include <QDateTime>
#include <QDir>
#include <QThread>
#include <exception>
#include <csignal>
#include <cstdio>
#ifdef Q_OS_WIN
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#  include <wincrypt.h>
#  pragma comment(lib, "Crypt32.lib")
#endif
#include <QSqlQuery>
#include <QByteArray>
#include <windows.h>
#include <QPasswordDigestor>
#include <QElapsedTimer>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
}

// ======================================================================
// core/camera_worker.h
// ======================================================================
class RecordingIndex;

struct CameraConfig {
    QString id;                 // уникальный id камеры: попадает в пути файлов и в БД
    QString name;               // отображаемое имя
    QString url;                // rtsp://login:password@host:554/...
    QString storageRoot;        // корневая папка архива
    QString dbPath;             // путь к SQLite-файлу с индексом
    int segmentSeconds = 300;   // длительность одного сегмента
    int ioTimeoutMs = 10000;    // таймаут открытия и чтения потока
};

// Один поток на камеру: читает пакеты через libavformat и пишет их БЕЗ перекодирования
// в mkv-сегменты фиксированной длительности. Сегмент всегда начинается с ключевого кадра.
// Метаданные сегментов попадают в SQLite (RecordingIndex).
// При обрыве потока переподключается с нарастающей паузой (1 с ... 30 с).
class CameraWorker : public QThread {
    Q_OBJECT
public:
    enum class State { Stopped, Connecting, Recording, Reconnecting };
    Q_ENUM(State)

    explicit CameraWorker(CameraConfig cfg, QObject *parent = nullptr);
    ~CameraWorker() override;

    const CameraConfig &config() const { return m_cfg; }

    void startRecording();   // вместо start(): сбрасывает флаг остановки
    void stop();             // не блокирует; после него вызвать wait()

signals:
    void stateChanged(const QString &cameraId, CameraWorker::State state);
    void segmentStarted(const QString &cameraId, const QString &path, qint64 startMs);
    void segmentFinished(const QString &cameraId, const QString &path,
                         qint64 startMs, qint64 endMs, qint64 sizeBytes);
    void errorOccurred(const QString &cameraId, const QString &message);

protected:
    void run() override;

private:
    void recordSession(RecordingIndex &index, bool &gotData);
    QString segmentPath(qint64 startMs) const;
    void armDeadline(int ms);
    void sleepInterruptible(int ms);
    static int interruptCallback(void *opaque);

    CameraConfig m_cfg;
    std::atomic<bool> m_stop{false};
    std::atomic<qint64> m_deadlineMs{0};
};

// ======================================================================
// core/i18n.h
// ======================================================================
// Язык интерфейса: русский или английский.
// Исходные тексты в коде написаны по-русски: T("Текст") возвращает его же (русский язык)
// или английский перевод из словаря (core/i18n.cpp). Если перевода нет — остаётся русский текст.
namespace I18n {

enum class Lang { Ru, En };

void init();                               // язык из настроек, иначе по языку системы
Lang language();
QString code();                            // "ru" или "en"
void setLanguage(Lang lang);               // запоминает выбор и язык календарей/дат
void setLanguageByCode(const QString &code);

// Смена языка пересоздаёт окно программы (запись камер при этом не прерывается)
void requestRestart();
bool takeRestartRequest();

QString translate(const char *ru);

} // namespace I18n

inline QString T(const char *ru)
{
    return I18n::translate(ru);
}

// ======================================================================
// app/camera_manager.h
// ======================================================================
class QTimer;

struct CameraEntry {
    QString id;
    QString name;
    QString group;
    QString url;               // основной поток: по нему идёт запись в архив
    QString liveUrl;           // подпоток для мозаики; может быть пустым
    bool record = true;        // вести запись
    int viewQuality = 0;       // качество просмотра: 0 — авто, 1 — основной поток, 2 — подпоток
    int brightness = 0;        // яркость картинки, от -50 до 50
    bool showTime = true;      // показывать дату и время на камере
    bool audio = false;        // включать звук при показе
    bool fillCell = false;   // заполнять ячейку целиком с обрезкой лишних краёв

    // PTZ по ONVIF (заполняется автоматически при импорте по ONVIF, если камера поворотная)
    QString ptzXAddr;
    QString ptzToken;
    QString ptzUser;
    QString ptzPassword;
    bool hasPtz() const { return !ptzXAddr.isEmpty() && !ptzToken.isEmpty(); }
};

inline QString stateText(CameraWorker::State s)
{
    switch (s) {
    case CameraWorker::State::Recording:    return T("Запись идёт");
    case CameraWorker::State::Connecting:   return T("Подключение…");
    case CameraWorker::State::Reconnecting: return T("Нет связи, переподключение…");
    case CameraWorker::State::Stopped:      break;
    }
    return T("Остановлено");
}

// Список камер (в QSettings), их рекордеры и проверка доступности.
// Рекордеры запускаются по очереди (по одному каждые ~120 мс), чтобы сотни камер не подключались
// одновременно. Если в настройках отключён автозапуск, при старте программы ничего не запускается.
class CameraManager : public QObject {
    Q_OBJECT
public:
    explicit CameraManager(QObject *parent = nullptr);
    ~CameraManager() override;

    QString storageRoot() const { return m_storageRoot; }
    QString dbPath() const { return m_storageRoot + QStringLiteral("/index.sqlite"); }

    const QList<CameraEntry> &cameras() const { return m_cameras; }
    const CameraEntry *find(const QString &id) const;
    CameraWorker::State stateOf(const QString &id) const;
    bool isStarted(const QString &id) const;       // рекордер запущен или стоит в очереди
    bool isOnline(const QString &id) const;        // камера отвечает по сети
    int onlineCount() const;
    int offlineCount() const;
    int recordingCount() const;

    QString add(CameraEntry e);                    // возвращает сгенерированный id
    void addMany(QList<CameraEntry> list);
    void update(const CameraEntry &e);
    void setLiveUrl(const QString &id, const QString &url);
    void remove(const QString &id);

    void startCamera(const QString &id);
    void stopCamera(const QString &id);
    void startAll();
    void stopAll();
    void setStorageRoot(const QString &path);      // новая папка записей; запущенные рекордеры перезапускаются
    void probeNow();                               // проверить, какие камеры в сети
    void shutdown();
    void enforceDemoLimit();

signals:
    void camerasChanged();
    void cameraStateChanged(const QString &id, CameraWorker::State state);
    void cameraError(const QString &id, const QString &message);
    void onlineChanged();

private:
    void load();
    void save() const;
    void syncWorker(const CameraEntry &e);
    void stopWorker(const QString &id);
    void startNext();
    void pumpProbe();

    QList<CameraEntry> m_cameras;
    QHash<QString, CameraWorker *> m_workers;
    QHash<QString, CameraWorker::State> m_states;
    QStringList m_startQueue;
    QTimer *m_startTimer = nullptr;
    QString m_storageRoot;

    QTimer *m_probeTimer = nullptr;
    QStringList m_probeQueue;
    QHash<QString, bool> m_probeResults;
    QHash<QString, bool> m_online;
    int m_probeActive = 0;
    bool m_probing = false;
};

// ======================================================================
// app/analog_camera_dialog.h
// ======================================================================
class QComboBox;
class QLineEdit;

// Аналоговая камера, подключённая через карту видеозахвата (USB- или PCI-капкарта): Windows видит
// её как обычное видеоустройство. Список устройств берётся из Qt (QMediaDevices::videoInputs()).
// Ограничение: запись в архив для таких камер не ведётся, доступен только живой просмотр —
// у программы нет RTSP-потока с капкарты, а запись устроена через захват именно RTSP (core/camera_worker).
class AnalogCameraDialog : public QDialog {
public:
    explicit AnalogCameraDialog(QWidget *parent = nullptr);

    CameraEntry entry() const;
    void accept() override;

private:
    QLineEdit *m_name = nullptr;
    QLineEdit *m_group = nullptr;
    QComboBox *m_device = nullptr;
};

// ======================================================================
// app/app_info.h
// ======================================================================
namespace App {

inline constexpr const char *kVersion = "1.0";
inline constexpr const char *kAuthor = "P.Sveshnikov";

inline QString title()
{
    return QString::fromUtf8("PSVMS");
}

// Выход из учётной записи: окно закрывается, затем снова спрашивается вход
inline bool &logoutFlag()
{
    static bool f = false;
    return f;
}
inline void requestLogout()
{
    logoutFlag() = true;
    QCoreApplication::quit();
}
inline bool takeLogoutRequest()
{
    const bool r = logoutFlag();
    logoutFlag() = false;
    return r;
}

} // namespace App

// ======================================================================
// app/camera_dialog.h
// ======================================================================
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QSlider;

// Настройки камеры: название, группа, потоки записи и просмотра, качество, яркость, время, звук.
// Используется и при добавлении вручную, и при правом щелчке по камере.
class CameraDialog : public QDialog {
public:
    CameraDialog(const CameraEntry &base, const QStringList &groups, QWidget *parent = nullptr);

    CameraEntry entry() const;
    void accept() override;

private:
    QWidget *withPicker(QLineEdit *edit, QComboBox *combo);
    void rebuildPickers();

    CameraEntry m_base;
    QLineEdit *m_name = nullptr;
    QComboBox *m_group = nullptr;
    QLineEdit *m_url = nullptr;
    QLineEdit *m_liveUrl = nullptr;
    QComboBox *m_urlPick = nullptr;
    QComboBox *m_livePick = nullptr;
    QComboBox *m_quality = nullptr;
    QSlider *m_brightness = nullptr;
    QLabel *m_brightLabel = nullptr;
    QCheckBox *m_record = nullptr;
    QCheckBox *m_showTime = nullptr;
    QCheckBox *m_audio = nullptr;
};

// ======================================================================
// app/common.h
// ======================================================================
// MIME-типы перетаскивания
inline constexpr const char *kCameraMime = "application/x-psvms-camera";   // камера из списка в ячейку
inline constexpr const char *kCellMime = "application/x-psvms-cell";       // ячейка на другую ячейку (обмен местами)

// ======================================================================
// app/camera_tree.h
// ======================================================================
// Дерево «группа → камера». Элемент камеры хранит id в Qt::UserRole
// и при перетаскивании отдаёт его в MIME kCameraMime.
class CameraTree : public QTreeWidget {
public:
    using QTreeWidget::QTreeWidget;

protected:
    QMimeData *mimeData(const QList<QTreeWidgetItem *> &items) const override
    {
        for (const QTreeWidgetItem *it : items) {
            const QString id = it->data(0, Qt::UserRole).toString();
            if (!id.isEmpty()) {
                auto *m = new QMimeData;
                m->setData(QString::fromLatin1(kCameraMime), id.toUtf8());
                return m;
            }
        }
        return nullptr;
    }
};

// ======================================================================
// app/cloud_dialog.h
// ======================================================================
class QComboBox;
class QLabel;
class QLineEdit;

// Облако Hik-Connect (кнопка справа сверху). Вход сохраняет только название аккаунта;
// пароль на диске не хранится.
class CloudDialog : public QDialog {
public:
    explicit CloudDialog(QWidget *parent = nullptr);

private:
    QComboBox *m_region = nullptr;
    QLineEdit *m_account = nullptr;
    QLineEdit *m_password = nullptr;
    QLabel *m_status = nullptr;
};

// ======================================================================
// app/credentials_dialog.h
// ======================================================================
class QWidget;

// Запрашивает логин и пароль у устройства, которое отказало во входе. false — пользователь пропустил.
bool askCredentials(QWidget *parent, const QString &address, QString *user, QString *password);

// ======================================================================
// app/devices_page.h
// ======================================================================
class CameraManager;
class QPushButton;
class QTableWidget;
class QToolButton;

// Устройства: список камер с состоянием связи и записи, добавление разными способами,
// запуск и остановка записи.
class DevicesPage : public QWidget {
    Q_OBJECT
public:
    explicit DevicesPage(CameraManager *mgr, QWidget *parent = nullptr);

public slots:
    void addCameraManually();

private slots:
    void rebuild();
    void refreshStates();
    void editCamera();
    void removeCamera();
    void startSelected();
    void stopSelected();
    void updateButtons();

private:
    QString currentId() const;
    QStringList selectedIds() const;

    CameraManager *m_mgr;
    QTableWidget *m_table = nullptr;
    QToolButton *m_addBtn = nullptr;
    QPushButton *m_edit = nullptr;
    QPushButton *m_remove = nullptr;
    QPushButton *m_removeAll = nullptr;
};

// ======================================================================
// app/events_page.h
// ======================================================================
class QTableWidget;

// Журнал событий: потеря связи, ошибки записи, смена состояния камер.
// Позже сюда же попадут события детектора движения.
class EventsPage : public QWidget {
    Q_OBJECT
public:
    explicit EventsPage(QWidget *parent = nullptr);

    void addEvent(const QString &camera, const QString &text);

private:
    QTableWidget *m_table = nullptr;
};

// ======================================================================
// app/fullscreen_view.h
// ======================================================================
class QLabel;
class QMediaPlayer;
class QAudioOutput;
class QTimer;
class QToolButton;
class VideoSurface;
class VideoCell;

// Камера на весь экран (двойной щелчок по ячейке). Всегда основной поток.
// Выход: Esc, двойной щелчок или кнопка ✕. Колесо мыши увеличивает картинку.
class FullscreenView : public QWidget {
public:
    explicit FullscreenView(VideoCell *cell, QWidget *parent = nullptr);
    void openOnScreenOf(QWidget *w);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void closeEvent(QCloseEvent *e) override;

private:
    VideoCell *m_cell = nullptr;
    VideoSurface *m_surface = nullptr;
    QLabel *m_hint = nullptr;
    QLabel *m_logo = nullptr; // фирменный знак в пустой ячейке
    QToolButton *m_close = nullptr;
};

// ======================================================================
// core/device_adder.h
// ======================================================================
struct AddedChannel {
    int number = 1;
    QString name;
    QString mainUrl;    // основной поток (для записи)
    QString subUrl;     // подпоток (для мозаики), может быть пустым
};

struct AddRequest {
    QString address;
    quint16 httpPort = 80;
    quint16 rtspPort = 554;
    QString user;
    QString password;
    int channels = 0;   // сколько каналов у регистратора, если определить автоматически не удастся
};

struct AddResult {
    bool ok = false;
    bool authFailed = false;     // нужен (другой) логин или пароль
    QString error;
    QString model;
    QString name;
    QString type;                // IPCamera, NVR, DVR ... (если известен)
    bool recorder = false;
    QString method;              // "ISAPI" или "RTSP"
    QList<AddedChannel> channels;
};

// Добавление камер и регистраторов Hikvision / HiWatch / Polyvision по IP.
// 1) Новые модели: ISAPI (модель, тип, каналы, номера потоков).
// 2) Старые модели и другие бренды: подбор рабочего RTSP-пути (core/rtsp_probe).
class DeviceAdder : public QObject {
    Q_OBJECT
public:
    explicit DeviceAdder(QObject *parent = nullptr);
    void start(const QList<AddRequest> &requests, int concurrency = 6);
    void cancel();

signals:
    void resolved(int index, const AddResult &result);
    void progress(int done, int total);
    void finished();

private:
    void pump();
    void runOne(int idx);
    void probeRtsp(int idx, const AddRequest &rq, const QString &prevError, int gen);
    void finishOne(int idx, const AddResult &r, int gen);

    QList<AddRequest> m_reqs;
    int m_next = 0;
    int m_done = 0;
    int m_active = 0;
    int m_conc = 6;
    int m_gen = 0;
    bool m_running = false;
};

// ======================================================================
// app/hik_ip_dialog.h
// ======================================================================
class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QSpinBox;

// Добавление камеры или регистратора Hikvision / HiWatch по IP-адресу (новые и старые модели).
// Если устройство просит пароль, программа его запрашивает.
class HikIpDialog : public QDialog {
public:
    explicit HikIpDialog(CameraManager *mgr, QWidget *parent = nullptr);

private:
    void start(const QString &user, const QString &password);
    void onResolved(const AddResult &res);

    CameraManager *m_mgr;
    DeviceAdder *m_adder = nullptr;
    QLineEdit *m_ip = nullptr;
    QSpinBox *m_httpPort = nullptr;
    QSpinBox *m_rtspPort = nullptr;
    QLineEdit *m_user = nullptr;
    QLineEdit *m_pass = nullptr;
    QSpinBox *m_channels = nullptr;
    QLineEdit *m_group = nullptr;
    QLineEdit *m_name = nullptr;
    QLabel *m_status = nullptr;
    QProgressBar *m_progress = nullptr;
    QPushButton *m_btn = nullptr;
    QString m_address;
    QString m_curUser;
};

// ======================================================================
// core/portscan.h
// ======================================================================
// Быстрая проверка, у каких адресов диапазона открыт TCP-порт (например, RTSP 554).
class PortScanner : public QObject {
    Q_OBJECT
public:
    explicit PortScanner(QObject *parent = nullptr);
    void start(quint32 firstIp, quint32 lastIp, quint16 port, int concurrency = 64, int timeoutMs = 800);
    void stop();

signals:
    void found(const QString &ip);
    void progress(int done, int total);
    void finished();

private:
    void pump();

    qint64 m_next = 0;
    qint64 m_last = -1;
    quint16 m_port = 554;
    int m_conc = 64;
    int m_timeout = 800;
    int m_active = 0;
    int m_done = 0;
    int m_total = 0;
    int m_gen = 0;
    bool m_running = false;
};

// ======================================================================
// app/icsee_import_dialog.h
// ======================================================================
class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QSpinBox;
class QTableWidget;

// Добавление камер iCSee (Xiongmai): сканируем диапазон IP на открытый RTSP-порт,
// отмечаем найденные и добавляем. RTSP-адреса строятся по шаблону iCSee:
// основной поток (stream=0) для записи и подпоток (stream=1) для мозаики.
// Для одной камеры укажите одинаковые адреса «от» и «до».
class IcseeImportDialog : public QDialog {
    Q_OBJECT
public:
    explicit IcseeImportDialog(CameraManager *mgr, QWidget *parent = nullptr);

private slots:
    void startScan();
    void addHost(const QString &ip);
    void addSelected();
    void setAllChecked(bool on);

private:
    CameraManager *m_mgr;
    PortScanner *m_scanner = nullptr;

    QLineEdit *m_from = nullptr;
    QLineEdit *m_to = nullptr;
    QSpinBox *m_port = nullptr;
    QLineEdit *m_user = nullptr;
    QLineEdit *m_pass = nullptr;
    QLineEdit *m_group = nullptr;
    QTableWidget *m_table = nullptr;
    QProgressBar *m_progress = nullptr;
    QLabel *m_status = nullptr;
    QPushButton *m_btnScan = nullptr;
    QPushButton *m_btnAdd = nullptr;

    QStringList m_hosts;          // строка таблицы i == m_hosts[i]
    QSet<QString> m_existing;     // уже добавленные камеры (по IP)
};

// ======================================================================
// core/hikvision.h
// ======================================================================
class QNetworkAccessManager;
class QTimer;
class QUdpSocket;

// Hikvision и её бренд HiWatch используют одни и те же протоколы:
//   RTSP:  rtsp://логин:пароль@IP:554/Streaming/Channels/<канал*100 + поток>
//          поток 1 — основной, 2 — дополнительный (канал 1: 101 и 102, канал 2: 201 и 202, ...)
//   ISAPI: http://IP/ISAPI/System/deviceInfo и /ISAPI/Streaming/channels (HTTP Digest)
inline QString hikRtspUrl(const QString &host, quint16 port, const QString &user, const QString &pass,
                          int channel, int stream)
{
    QUrl u;
    u.setScheme(QStringLiteral("rtsp"));
    u.setHost(host);
    u.setPort(port);
    u.setUserName(user);
    u.setPassword(pass);
    u.setPath(QStringLiteral("/Streaming/Channels/%1").arg(channel * 100 + stream));
    return u.toString(QUrl::FullyEncoded);
}

struct HikChannel {
    int number = 1;
    QString name;
};

struct HikDevice {
    QString address;
    quint16 httpPort = 80;
    QString model;
    QString name;
    QString serial;
    QString type;           // IPCamera, NVR, DVR ...
    bool activated = true;
    QList<HikChannel> channels;

    bool isRecorder() const
    {
        return type.contains(QLatin1String("NVR"), Qt::CaseInsensitive)
               || type.contains(QLatin1String("DVR"), Qt::CaseInsensitive);
    }
};

struct HikRequest {
    QString address;
    quint16 httpPort = 80;
    QString user;
    QString password;
    int fallbackChannels = 0;   // сколько каналов у регистратора, если список получить не удалось
};

struct HikResult {
    bool ok = false;
    bool authFailed = false;     // нужен (другой) логин или пароль
    QString error;
    HikDevice device;
};

// Производитель по модели или названию: "Hikvision", "HiWatch", "Polyvision" или пустая строка
QString guessVendor(const QString &model, const QString &name = QString());

// Поиск устройств по протоколу SADP (multicast 239.255.255.250:37020) — как в программе SADP Tool.
// Работает в своей подсети. Ответ зависит от прошивки, поэтому для других подсетей
// используйте сканирование диапазона IP.
class HikDiscovery : public QObject {
    Q_OBJECT
public:
    explicit HikDiscovery(QObject *parent = nullptr);
    void start(int timeoutMs = 4000);
    void stop();

signals:
    void deviceFound(const HikDevice &dev);
    void finished();

private:
    void sendProbes();
    void onReadyRead();

    QUdpSocket *m_socket = nullptr;
    QTimer *m_timer = nullptr;
    QSet<QString> m_seen;
    bool m_running = false;
};

// Пакетный опрос по ISAPI: тип устройства, модель и список каналов (камера = 1 канал,
// регистратор = все его каналы). Одновременно опрашивается не более `concurrency` устройств.
class HikResolver : public QObject {
    Q_OBJECT
public:
    explicit HikResolver(QObject *parent = nullptr);
    void start(const QList<HikRequest> &requests, int concurrency = 8);
    void cancel();

signals:
    void resolved(int index, const HikResult &result);
    void progress(int done, int total);
    void finished();

private:
    void pump();
    void runJob(int idx);
    void jobDone(int idx, const HikResult &r, int gen);

    QNetworkAccessManager *m_nam = nullptr;
    QList<HikRequest> m_requests;
    int m_next = 0;
    int m_done = 0;
    int m_active = 0;
    int m_conc = 8;
    int m_gen = 0;
    bool m_running = false;
};

// ======================================================================
// core/onvif.h
// ======================================================================
class QNetworkAccessManager;
class QTimer;
class QUdpSocket;

// Найденное ONVIF-устройство.
struct OnvifDevice {
    QString address;    // IPv4 камеры
    QString xaddr;      // http://ip:port/onvif/device_service
    QString name;       // из scopes (если камера сообщила)
    QString hardware;   // модель из scopes
};

struct OnvifRequest {
    QString xaddr;
    QString user;
    QString password;
};

// Результат опроса камеры: адреса потоков уже с логином и паролем.
struct OnvifStreams {
    bool ok = false;
    QString error;
    QString mainUrl;   // профиль с максимальным разрешением (для записи)
    QString subUrl;    // профиль с минимальным разрешением (для мозаики), может быть пустым

    // Для PTZ (может отсутствовать, если у камеры нет поворотного механизма):
    QString ptzXAddr;       // адрес PTZ-сервиса камеры
    QString ptzToken;       // токен профиля, которым управляем
    bool hasPtz = false;
};

// ---------------------------------------------------------------------------
// Поиск по WS-Discovery (multicast 239.255.255.250:3702).
// Работает только в той же подсети (L2), что и компьютер.
class OnvifDiscovery : public QObject {
    Q_OBJECT
public:
    explicit OnvifDiscovery(QObject *parent = nullptr);
    void start(int timeoutMs = 4000);
    void stop();

signals:
    void deviceFound(const OnvifDevice &dev);
    void finished();

private:
    void sendProbes();
    void onReadyRead();

    QUdpSocket *m_socket = nullptr;
    QTimer *m_timer = nullptr;
    QSet<QString> m_seen;
    bool m_running = false;
};

// ---------------------------------------------------------------------------
// Сканирование диапазона IP: на каждый адрес уходит GetSystemDateAndTime
// (разрешён без авторизации). Находит камеры в других подсетях и VLAN,
// куда multicast не доходит.
class OnvifScanner : public QObject {
    Q_OBJECT
public:
    explicit OnvifScanner(QObject *parent = nullptr);
    void start(quint32 firstIp, quint32 lastIp, quint16 port = 80, int concurrency = 64);
    void stop();

signals:
    void deviceFound(const OnvifDevice &dev);
    void progress(int done, int total);
    void finished();

private:
    void pump();

    QNetworkAccessManager *m_nam = nullptr;
    qint64 m_next = 0;
    qint64 m_last = -1;
    quint16 m_port = 80;
    int m_conc = 64;
    int m_active = 0;
    int m_done = 0;
    int m_total = 0;
    int m_gen = 0;
    bool m_running = false;
};

// ---------------------------------------------------------------------------
// Пакетное получение RTSP-адресов: для каждой камеры по цепочке
// GetSystemDateAndTime -> GetCapabilities -> GetProfiles -> GetStreamUri.
// Одновременно опрашивается не более `concurrency` камер.
class OnvifResolver : public QObject {
    Q_OBJECT
public:
    explicit OnvifResolver(QObject *parent = nullptr);
    void start(const QList<OnvifRequest> &requests, int concurrency = 8);
    void cancel();

signals:
    void resolved(int index, const OnvifStreams &streams);   // index — позиция в requests
    void progress(int done, int total);
    void finished();

private:
    void pump();
    void runJob(int idx);
    void jobDone(int idx, const OnvifStreams &s, int gen);

    QNetworkAccessManager *m_nam = nullptr;
    QList<OnvifRequest> m_requests;
    int m_next = 0;
    int m_done = 0;
    int m_active = 0;
    int m_conc = 8;
    int m_gen = 0;
    bool m_running = false;
};

// ======================================================================
// app/lan_search_dialog.h
// ======================================================================
class QComboBox;
class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QSpinBox;
class QTableWidget;

// Поиск камер и регистраторов Hikvision, HiWatch и Polyvision (новые и старые модели) в локальной сети:
//   SADP (Hikvision/HiWatch) + ONVIF WS-Discovery (все бренды) + сканирование диапазона IP.
// Для найденных устройств подбираются адреса потоков; если нужен пароль, программа его спрашивает.
class LanSearchDialog : public QDialog {
    Q_OBJECT
public:
    explicit LanSearchDialog(CameraManager *mgr, QWidget *parent = nullptr);

private slots:
    void startSearch();
    void startScan();
    void importSelected();
    void applyFilter();
    void setAllChecked(bool on);

private:
    struct Candidate {
        QString address;
        quint16 httpPort = 80;
        QString model;
        QString vendor;
    };

    void addCandidate(const QString &ip, quint16 httpPort, const QString &model, const QString &name);
    void setState(int row, const QString &text);
    void setBusy(bool busy);
    void runRound();
    void onRoundFinished();
    void finishImport();
    void addEntries(int row, const AddResult &res);

    CameraManager *m_mgr;
    HikDiscovery *m_sadp = nullptr;
    OnvifDiscovery *m_onvif = nullptr;
    PortScanner *m_scan = nullptr;
    DeviceAdder *m_adder = nullptr;

    QComboBox *m_filter = nullptr;
    QLineEdit *m_from = nullptr;
    QLineEdit *m_to = nullptr;
    QSpinBox *m_rtsp = nullptr;
    QLineEdit *m_user = nullptr;
    QLineEdit *m_pass = nullptr;
    QLineEdit *m_group = nullptr;
    QSpinBox *m_channels = nullptr;
    QTableWidget *m_table = nullptr;
    QProgressBar *m_progress = nullptr;
    QLabel *m_status = nullptr;
    QPushButton *m_btnFind = nullptr;
    QPushButton *m_btnScan = nullptr;
    QPushButton *m_btnImport = nullptr;

    QList<Candidate> m_candidates;     // строка таблицы i == m_candidates[i]
    QSet<QString> m_existing;          // адреса, уже добавленные в программу
    int m_pendingSearches = 0;

    // состояние добавления (камеры опрашиваются «раундами»: после запроса пароля — новый раунд)
    QList<int> m_roundRows;
    QHash<int, QString> m_rowUser;
    QHash<int, QString> m_rowPass;
    QList<int> m_authRetry;
    QList<CameraEntry> m_entries;
    QList<int> m_addedRows;
    int m_failed = 0;
    int m_devicesOk = 0;
    QString m_commonUser;
    QString m_commonPass;
    QString m_groupName;
    quint16 m_rtspPort = 554;
    int m_fallbackChannels = 0;
};

// ======================================================================
// app/live_view_page.h
// ======================================================================
class CameraManager;
class CameraTree;
class QLineEdit;
class QPropertyAnimation;
class QToolButton;
class QTreeWidgetItem;
class VideoGrid;

// Мониторинг: слева список камер и PTZ, справа мозаика. Внизу панель: раскладка (до 64 ячеек),
// шаблоны, архив, запуск камер.
class LiveViewPage : public QWidget {
    Q_OBJECT
public:
    explicit LiveViewPage(CameraManager *mgr, QWidget *parent = nullptr);
    VideoGrid *grid() const { return m_grid; }
    void openArchiveForSelectedCamera();

signals:
    void addCameraRequested();
    void message(const QString &text);

private slots:
    void rebuildTree();
    void refreshStates();
    void applyFilter(const QString &text);
    void setPanelVisible(bool on);

private:
    void openArchive(const QString &cameraId);
    void rebuildTemplatesMenu();

    CameraManager *m_mgr;
    CameraTree *m_tree = nullptr;
    QLineEdit *m_search = nullptr;
    VideoGrid *m_grid = nullptr;
    QWidget *m_leftPanel = nullptr;
    QToolButton *m_layoutBtn = nullptr;
    QToolButton *m_tplBtn = nullptr;
    QPointer<QPropertyAnimation> m_panelAnim;
    QHash<QString, QTreeWidgetItem *> m_items;
};

// ======================================================================
// app/login_dialog.h
// ======================================================================
class QWidget;

// Вход в программу. Пароль не обязателен: при первом запуске создаётся пользователь «Администратор»
// без пароля, и вход выполняется автоматически. Окно выбора пользователя и пароля показывается,
// только если есть пароль или несколько пользователей и нет запомненного входа.
// Возвращает false, если пользователь закрыл окно входа — программу нужно закрыть.
bool authenticate(QWidget *parent = nullptr);

// Смена пароля текущего пользователя (пустой пароль — вход без пароля).
void changeOwnPassword(QWidget *parent);

// ======================================================================
// app/main_window.h
// ======================================================================
class CameraManager;
class DevicesPage;
class EventsPage;
class LiveViewPage;
class QLabel;
class QStackedWidget;
class SettingsPage;
class SideNav;
class TopBar;
class CameraStatusDialog;

// Проверка встроенного пароля разработчика. В исходнике хранится только PBKDF2-хэш,
// исходный пароль не записывается ни в настройки, ни в файлы программы.

// Окно PSVMS: слева постоянно раскрытое меню (Мониторинг, Устройства, События, Настройки, Язык),
// сверху панель со счётчиками камер и кнопкой облака Hik-Connect, внизу строка состояния.
namespace {
QIcon makeAppIcon();
QIcon makeCellIcon(const QString &kind, const QColor &color = QColor(255, 224, 104));
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(CameraManager *mgr, QWidget *parent = nullptr);
    bool startFullscreen() const;

protected:
    void closeEvent(QCloseEvent *e) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void resetInactivityTimer();
    void showLanguageMenu();
    void toggleMainFullscreen();
    void leaveMainFullscreen();
    void applyLanguage(const QString &code);
    void updateLanguageItem();
    QString pageTitle(int page) const;
    void toggleVoiceControl();
    void handleVoiceCommand(const QString &command);
    void stopVoiceControl();

    CameraManager *m_mgr;
    SideNav *m_nav = nullptr;
    TopBar *m_top = nullptr;
    QStackedWidget *m_stack = nullptr;
    LiveViewPage *m_live = nullptr;
    DevicesPage *m_devices = nullptr;
    EventsPage *m_events = nullptr;
    SettingsPage *m_settings = nullptr;
    QPointer<CameraStatusDialog> m_cameraStatusDialog;
    QTimer *m_inactivityTimer = nullptr;
    QLabel *m_clock = nullptr;
    QLabel *m_sysLabel = nullptr;
    QPushButton *m_voiceButton = nullptr;
    QProcess *m_voiceProcess = nullptr;
    QByteArray m_voiceOutputBuffer;
    int m_langItem = -1;
    int m_settingsItem = -1;
    int m_fullscreenItem = -1;
    QRect m_preFullscreenGeometry;
    bool m_preFullscreenMaximized = false;
    bool m_hasPreFullscreenState = false;
};

// ======================================================================
// app/onvif_import_dialog.h
// ======================================================================
class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QSpinBox;
class QTableWidget;

// Массовое добавление камер по ONVIF:
//   1. найти устройства (multicast в своей подсети или сканирование диапазона IP),
//   2. указать общий логин и пароль,
//   3. отметить нужные и нажать «Добавить»: программа сама получит RTSP-адреса
//      основного потока (для записи) и подпотока (для мозаики).
class OnvifImportDialog : public QDialog {
    Q_OBJECT
public:
    explicit OnvifImportDialog(CameraManager *mgr, QWidget *parent = nullptr);

private slots:
    void startDiscovery();
    void startScan();
    void addDevice(const OnvifDevice &dev);
    void importSelected();
    void setAllChecked(bool on);

private:
    QString displayName(int row) const;
    void setBusy(bool busy);

    CameraManager *m_mgr;
    OnvifDiscovery *m_discovery = nullptr;
    OnvifScanner *m_scanner = nullptr;
    OnvifResolver *m_resolver = nullptr;

    QLineEdit *m_from = nullptr;
    QLineEdit *m_to = nullptr;
    QSpinBox *m_port = nullptr;
    QLineEdit *m_user = nullptr;
    QLineEdit *m_pass = nullptr;
    QLineEdit *m_group = nullptr;
    QTableWidget *m_table = nullptr;
    QProgressBar *m_progress = nullptr;
    QLabel *m_status = nullptr;
    QPushButton *m_btnDiscover = nullptr;
    QPushButton *m_btnScan = nullptr;
    QPushButton *m_btnImport = nullptr;

    QList<OnvifDevice> m_devices;        // строка таблицы i == m_devices[i]
    QSet<QString> m_existingHosts;       // уже добавленные камеры (по IP)
    QList<int> m_importRows;             // строки, ушедшие в опрос (по порядку запросов)
    QList<CameraEntry> m_pending;        // успешно опрошенные камеры
    QList<int> m_pendingRows;
    int m_failed = 0;
};

// ======================================================================
// core/recording_index.h
// ======================================================================
struct SegmentInfo {
    qint64 id = -1;
    QString cameraId;
    QString path;
    qint64 startMs = 0;   // UTC, мс от эпохи
    qint64 endMs = 0;     // 0, если сегмент ещё пишется
    qint64 sizeBytes = 0;
};

// Индекс сегментов архива в SQLite.
//
// ВАЖНО: соединение QSqlDatabase нельзя передавать между потоками, поэтому
// каждый объект RecordingIndex нужно создавать и вызывать open() в том потоке,
// где он будет использоваться (в CameraWorker::run() и отдельно в GUI-потоке).
// Несколько соединений к одному файлу работают штатно (включён режим WAL).
class RecordingIndex {
public:
    RecordingIndex() = default;
    ~RecordingIndex();
    RecordingIndex(const RecordingIndex &) = delete;
    RecordingIndex &operator=(const RecordingIndex &) = delete;

    bool open(const QString &dbPath);   // открывает соединение и создаёт схему
    void close();
    QString lastError() const { return m_lastError; }

    // Запись
    qint64 beginSegment(const QString &cameraId, const QString &path, qint64 startMs);
    bool endSegment(qint64 id, qint64 endMs, qint64 sizeBytes);
    // После сбоя/краша остаются сегменты без end_ms: восстановить по файлу или удалить.
    void closeDanglingSegments(const QString &cameraId);

    // Чтение (для архива и таймлайна)
    QList<SegmentInfo> segmentsInRange(const QString &cameraId, qint64 fromMs, qint64 toMs);
    SegmentInfo segmentAt(const QString &cameraId, qint64 ms);   // id == -1, если записи нет

private:
    QString m_conn;
    QString m_lastError;
};

// ======================================================================
// app/playback_page.h
// ======================================================================
class CameraManager;
class QAudioOutput;
class QCalendarWidget;
class QComboBox;
class QLabel;
class QListWidget;
class QMediaPlayer;
class QPushButton;
class QSlider;
class QVideoWidget;

// Архив: выбор камеры и даты, список сегментов из индекса SQLite, воспроизведение.
class PlaybackPage : public QWidget {
    Q_OBJECT
public:
    explicit PlaybackPage(CameraManager *mgr, QWidget *parent = nullptr);
    void selectCamera(const QString &id);   // выбрать камеру в списке

protected:
    void showEvent(QShowEvent *e) override;

private slots:
    void reloadCameras();
    void reloadSegments();
    void playCurrent();
    void togglePlay();

private:
    CameraManager *m_mgr;
    RecordingIndex m_index;   // соединение с БД живёт в GUI-потоке

    QComboBox *m_camera = nullptr;
    QCalendarWidget *m_calendar = nullptr;
    QListWidget *m_list = nullptr;
    QLineEdit *m_search = nullptr;
    QLabel *m_archiveCount = nullptr;
    QVideoWidget *m_video = nullptr;
    QMediaPlayer *m_player = nullptr;
    QAudioOutput *m_audio = nullptr;
    QPushButton *m_playBtn = nullptr;
    QSlider *m_slider = nullptr;
    QLabel *m_time = nullptr;
};

// ======================================================================
// app/playback_window.h
// ======================================================================
class CameraManager;

// Архив камеры в отдельном окне (из панели «Мониторинг» или меню камеры)
class PlaybackWindow : public QDialog {
public:
    PlaybackWindow(CameraManager *mgr, const QString &cameraId, QWidget *parent = nullptr);
};

// ======================================================================
// app/ptz_panel.h
// ======================================================================
class CameraManager;
class QNetworkAccessManager;
class QSlider;
class QToolButton;

// Панель управления поворотной камерой (PTZ) по ONVIF. Активна только для камеры, у которой PTZ
// определился при импорте по ONVIF (core/onvif.h, OnvifStreams::hasPtz); для остальных камер кнопки
// выключены. Направление — удерживая кнопку, камера движется; отпустили — останавливается.
class PtzPanel : public QWidget {
    Q_OBJECT
public:
    explicit PtzPanel(CameraManager *mgr, QWidget *parent = nullptr);

public slots:
    void setCamera(const QString &cameraId);   // пустая строка — камера не выбрана

private:
    void updateEnabled();

    CameraManager *m_mgr;
    QNetworkAccessManager *m_nam;
    QString m_cameraId;
    QToolButton *m_dirs[9] = {};
    QToolButton *m_zoomIn = nullptr;
    QToolButton *m_zoomOut = nullptr;
    QSlider *m_speed = nullptr;
    QWidget *m_note = nullptr;
};

// ======================================================================
// app/rtsp_quick_dialog.h
// ======================================================================
class QCheckBox;
class QLineEdit;

// Простое добавление камеры: только название и адрес RTSP-потока, без мастеров и поиска.
class RtspQuickDialog : public QDialog {
public:
    explicit RtspQuickDialog(QWidget *parent = nullptr);

    CameraEntry entry() const;
    void accept() override;

private:
    QLineEdit *m_name = nullptr;
    QLineEdit *m_url = nullptr;
    QLineEdit *m_group = nullptr;
    QCheckBox *m_record = nullptr;
};

// ======================================================================
// app/settings.h
// ======================================================================
// Сообщает окнам, что настройки изменились (частота кадров, формат времени и т. д.)
class CfgNotifier : public QObject {
    Q_OBJECT
signals:
    void changed();
};

namespace Cfg {

// Настройки просмотра (кэшируются: читаются на каждый кадр)
struct View {
    bool lowPower = false;      // режим для слабого компьютера
    int maxFps = 15;            // предел кадров в секунду на одну ячейку
    bool smooth = true;         // сглаживание картинки при масштабировании
    bool timestamp = true;      // дата и время на камерах
    QString timeFormat = QStringLiteral("dd.MM.yyyy HH:mm:ss");
    bool animations = true;     // плавные анимации интерфейса
    bool showInfo = true;       // название, FPS, разрешение и качество связи поверх картинки
};

const View &view();
void reload();                  // перечитать настройки и оповестить окна
CfgNotifier *notifier();

bool autostartCameras();        // запускать камеры при старте программы
QString videoDir();             // папка для записей
QString screenshotDir();        // папка для снимков экрана

} // namespace Cfg

// ======================================================================
// app/settings_page.h
// ======================================================================
class CameraManager;
class QLabel;
class QTableWidget;

// Настройки программы: общие, видео и папки, производительность, пользователи, резервная копия.
class SettingsPage : public QWidget {
    Q_OBJECT
public:
    explicit SettingsPage(CameraManager *mgr, QWidget *parent = nullptr);

signals:
    void languageChosen(const QString &code);   // "ru" или "en"
    void storageChanged();
    void proSessionActivated();
    void proSessionDeactivated();

private slots:
    void rebuildUsers();

private:
    QWidget *buildGeneralTab();
    QWidget *buildVideoTab();
    QWidget *buildPerformanceTab();
    QWidget *buildNetworkTab();
    QWidget *buildAiTab();
    QWidget *buildUsersTab();
    QWidget *buildBackupTab();
    QWidget *buildLicenseTab();
    QWidget *buildAboutTab();
    QString currentUserId() const;

    CameraManager *m_mgr;
    QTableWidget *m_users = nullptr;
    QLabel *m_licenseStatus = nullptr;
};

// ======================================================================
// app/side_nav.h
// ======================================================================
class QButtonGroup;
class QFrame;
class QLabel;
class QPropertyAnimation;
class QPushButton;
class QVBoxLayout;

// Боковое меню: иконка и название пункта, подсветка-«пилюля» скользит к выбранному пункту,
// меню постоянно отображается в полном виде. Пункты бывают двух видов: страницы (подсвечиваются)
// и действия (например, выбор языка), внизу — учётная запись пользователя.
class SideNav : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int navWidth READ navWidth WRITE setNavWidth)
public:
    explicit SideNav(QWidget *parent = nullptr);

    int addPage(const QString &icon, const QString &title);      // возвращает номер пункта
    int addAction(const QString &icon, const QString &title);
    int addBottomAction(const QString &icon, const QString &title);   // компактный пункт внизу (язык, настройки)
    void setItemText(int index, const QString &icon, const QString &title);
    void setItemVisible(int index, bool visible);
    QWidget *itemButton(int index) const;
    void setCurrent(int index, bool animate = true);
    int current() const { return m_current; }

    void setUser(const QString &name, const QString &role);
    bool isExpanded() const { return m_expanded; }
    void setExpanded(bool on, bool animate = true);

    int navWidth() const { return width(); }
    void setNavWidth(int w);

signals:
    void currentChanged(int index);       // выбрана страница
    void actionTriggered(int index);      // нажат пункт-действие
    void changePasswordRequested();
    void logoutRequested();

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

private:
    QPixmap m_wallpaper;
    struct Item {
        QString icon;
        QString title;
        QPushButton *button = nullptr;
        bool isPage = true;
    };

    int addItem(const QString &icon, const QString &title, bool isPage);
    void updatePill(bool animate);
    void applyTexts();
    void showUserMenu();

    QFrame *m_pill = nullptr;
    QLabel *m_brand = nullptr;
    QLabel *m_brandIcon = nullptr;
    QPushButton *m_userBtn = nullptr;
    QToolButton *m_toggleBtn = nullptr;
    QVBoxLayout *m_itemsLayout = nullptr;
    QVBoxLayout *m_bottomLayout = nullptr;
    QButtonGroup *m_group = nullptr;
    QList<Item> m_items;
    int m_current = -1;
    bool m_expanded = true;
    QString m_userName;
    QString m_userRole;
    QPointer<QPropertyAnimation> m_pillAnim;
    QPointer<QPropertyAnimation> m_widthAnim;
};

// ======================================================================
// app/splash.h
// ======================================================================
class QSplashScreen;

// Заставка «PSVMS» при запуске программы
QSplashScreen *createSplash();
void finishSplash(QSplashScreen *splash);   // плавно скрывает и удаляет заставку

// ======================================================================
// app/status_dialog.h
// ======================================================================
class CameraManager;
class QLabel;
class QTableWidget;

// Окно «Состояние камер»: сколько камер в сети и не в сети, сколько пишется, список с адресами.
class CameraStatusDialog : public QDialog {
public:
    CameraStatusDialog(CameraManager *mgr, QWidget *parent = nullptr);
    void showOfflineOnly();
    void showAllCameras();

private:
    void rebuild();

    CameraManager *m_mgr;
    QLabel *m_cards[4] = {nullptr, nullptr, nullptr, nullptr};
    QTableWidget *m_table = nullptr;
    bool m_offlineOnly = false;
};

// ======================================================================
// app/theme.h
// ======================================================================
// Дизайн в духе HikCentral Professional: плоские тёмно-серые панели, прямые углы,
// боковое меню с подсветкой-«пилюлей», заголовки панелей,
// нижняя панель инструментов под видео. Акцент: янтарно-жёлтый #f2c200.
inline QString appStyleSheet()
{
    return QStringLiteral(R"PSVMS(
* { font-family:"Segoe UI","Arial"; font-size:10pt; }
QWidget { color:#eee9dc; background:transparent; }
QMainWindow, QWidget#centralWidget, QStackedWidget { background:transparent; }
QWidget#sideNav { background:rgba(8,9,11,138); border-right:1px solid rgba(220,174,75,150); }
QWidget#sideNav QLabel, QWidget#sideNav QFrame { background:transparent; }
QLabel#navBrand { color:#e8b94f; font-size:22px; font-weight:900; letter-spacing:1.5px; }
QLabel#navUser { color:#e9dfc9; font-weight:700; }
QToolButton#navToggle { color:#f3d17c; background:rgba(8,9,10,180); border:1px solid rgba(218,174,82,170); border-radius:9px; }
QToolButton#navToggle:hover { background:rgba(75,54,20,210); border-color:#f4d58b; }
QPushButton#navItem, QPushButton#navBottomItem, QPushButton#navLang, QPushButton#navUserBtn {
 color:#f0eadf; background:rgba(8,10,12,110); border:1px solid rgba(190,151,73,38);
 border-radius:9px; min-height:43px; padding:5px 12px; text-align:left; font-weight:650;
}
QPushButton#navItem:hover, QPushButton#navBottomItem:hover, QPushButton#navLang:hover, QPushButton#navUserBtn:hover {
 color:#fff1c7; background:rgba(184,135,44,100); border-color:rgba(236,193,103,190);
}
QPushButton#navItem:checked {
 color:#1b160c; font-weight:850;
 background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 rgba(255,225,151,245),stop:0.55 rgba(211,163,68,235),stop:1 rgba(130,88,27,225));
 border:1px solid #ffe2a0;
}
QPushButton#navBottomItem[prominent="true"] { color:#f3d17c; background:rgba(155,108,33,55); border:1px solid rgba(215,170,80,150); min-height:46px; font-weight:750; }
QFrame#navPill { background:rgba(218,170,70,50); border:1px solid rgba(240,200,120,145); border-radius:9px; }
QWidget#topBar { background:rgba(7,8,10,205); border-bottom:1px solid rgba(210,166,77,175); }
QLabel#topTitle { color:#f1d28a; font-size:16px; font-weight:800; }
QLabel#clock { color:#f0d18a; background:rgba(193,145,57,25); border:1px solid #745723; border-radius:8px; padding:4px 10px; }
QLabel#sysStats { color:#c7bda9; padding:0 10px; }
QLabel#chipOnline { color:#b9f4c6; background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 rgba(35,112,72,190),stop:1 rgba(17,55,43,220)); border:1px solid rgba(105,220,145,175); border-radius:13px; padding:6px 14px; font-size:10pt; font-weight:800; }
QLabel#chipOffline { color:#ffd0b8; background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 rgba(145,75,48,190),stop:1 rgba(67,35,29,220)); border:1px solid rgba(237,145,104,175); border-radius:13px; padding:6px 14px; font-size:10pt; font-weight:800; }
QPushButton#cameraStatusBtn, QPushButton#cloudBtn { color:#f1d28a; background:rgba(24,22,18,220); border:1px solid #8c6a2d; border-radius:9px; padding:6px 13px; font-weight:750; }
QPushButton#cameraStatusBtn:hover, QPushButton#cloudBtn:hover { color:#fff0bd; background:rgba(74,53,24,230); border-color:#e7bd63; }
QStatusBar { color:#aaa59a; background:rgba(7,8,10,205); border-top:1px solid #725523; }
QStatusBar QLabel { background:transparent; color:#c7bda9; }
QLabel#panelTitle { color:#f2d28b; background:rgba(24,22,18,205); border-bottom:1px solid #80602b; padding:10px 14px; font-weight:800; }
QWidget#videoToolbar, QWidget#controlRail { background:rgba(8,9,11,190); border-top:1px solid rgba(190,145,60,150); }
QLabel#statCard { color:#f1dfb5; background:rgba(17,17,17,190); border:1px solid #69512b; border-radius:11px; padding:9px; }
QPushButton, QToolButton { color:#eee8da; font-weight:650; min-height:29px; padding:5px 11px; background:rgba(19,20,21,220); border:1px solid #6b542e; border-top-color:#987744; border-radius:8px; }
QPushButton:hover, QToolButton:hover { color:#fff0c3; background:rgba(57,43,23,235); border-color:#d3a84e; }
QPushButton:pressed, QToolButton:pressed, QToolButton:checked { color:#17130b; background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #ffe6a0,stop:0.5 #d4a64a,stop:1 #98691e); border-color:#ffe2a0; }
QPushButton:disabled, QToolButton:disabled { color:#6f6b62; border-color:#302a20; }
QLineEdit, QComboBox, QDateEdit, QSpinBox, QTimeEdit, QPlainTextEdit, QTextEdit { color:#eee8da; background:rgba(12,14,16,230); border:1px solid #51432b; border-top-color:#80653a; border-radius:7px; padding:6px 8px; selection-background-color:#c99a32; selection-color:#151311; }
QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QSpinBox:focus, QTextEdit:focus { border:1px solid #d4aa56; }
QComboBox QAbstractItemView { color:#eee8da; background:#171819; border:1px solid #80602b; selection-background-color:#c99a32; selection-color:#151311; }
QTreeWidget, QTableWidget, QListWidget { color:#e9e4d9; background:rgba(12,14,16,225); alternate-background-color:rgba(27,27,27,225); border:1px solid #51432b; gridline-color:#302a20; }
QTreeWidget::item, QListWidget::item { padding:5px 3px; }
QTreeWidget::item:selected, QTableWidget::item:selected, QListWidget::item:selected { color:#17130b; background:#c99a32; }
QHeaderView::section { color:#e9cb88; background:rgba(33,30,24,240); border:none; border-right:1px solid #51432b; border-bottom:1px solid #80602b; padding:8px; }
QGroupBox { color:#e9d6ad; background:rgba(18,20,22,210); border:1px solid #59472b; border-radius:9px; margin-top:14px; padding:10px; }
QGroupBox::title { color:#e7c477; subcontrol-origin:margin; left:11px; padding:0 5px; }
QTabWidget::pane { background:rgba(17,19,21,230); border:1px solid #5a482b; border-radius:8px; }
QTabBar::tab { color:#c9c1b1; background:rgba(25,26,27,225); border:1px solid #51432b; padding:8px 15px; margin-right:3px; border-top-left-radius:7px; border-top-right-radius:7px; }
QTabBar::tab:selected { color:#17130b; background:#d4aa56; border-color:#f0d28c; font-weight:800; }
QCheckBox::indicator { width:16px; height:16px; border:1px solid #80653a; border-radius:4px; background:#111315; }
QCheckBox::indicator:checked { background:#d4aa56; border-color:#f3d68e; }
QSlider::groove:horizontal { height:4px; background:#39332a; border-radius:2px; }
QSlider::handle:horizontal { width:14px; margin:-5px 0; border-radius:7px; background:#e0b65e; }
QProgressBar { color:#eee8da; background:#111315; border:1px solid #59472b; border-radius:6px; text-align:center; }
QProgressBar::chunk { background:#c99a32; border-radius:5px; }
QScrollBar:vertical { background:#111315; width:10px; }
QScrollBar:horizontal { background:#111315; height:10px; }
QScrollBar::handle:vertical, QScrollBar::handle:horizontal { background:#59472b; border-radius:5px; min-height:24px; min-width:24px; }
QScrollBar::handle:vertical:hover, QScrollBar::handle:horizontal:hover { background:#bd9140; }
QScrollBar::add-line, QScrollBar::sub-line { width:0; height:0; }
QSplitter::handle { background:rgba(61,48,32,180); }
QMenu { color:#eee8da; background:#171819; border:1px solid #80602b; padding:4px; }
QMenu::item { padding:7px 22px; }
QMenu::item:selected { color:#17130b; background:#d4aa56; }
QToolTip { color:#fff0c3; background:#171819; border:1px solid #a47b32; padding:6px 9px; border-radius:5px; }
VideoCell { background:rgba(7,8,9,205); border:1px solid #b18b4a; border-radius:9px; }
VideoCell[selected="true"] { border:2px solid #e4b956; }
VideoCell QWidget#cellBar { min-height:32px; background:rgba(8,9,10,220); border-bottom:1px solid #725523; border-top-left-radius:8px; border-top-right-radius:8px; }
VideoCell QLabel#cellTitle { color:#f0dfb8; background:transparent; padding:5px 10px; font-weight:750; }
VideoCell QWidget#cellIdle { background:rgba(12,13,14,190); border-bottom-left-radius:8px; border-bottom-right-radius:8px; }
VideoCell QLabel#placeholder { color:#c9b27c; background:transparent; padding:8px; font-weight:650; }
VideoCell QLabel#emptyCellBrand { background:transparent; }
VideoCell QToolButton#cellBtn { color:#f0d28d; background:rgba(10,11,12,210); border:1px solid #80602b; border-radius:7px; padding:2px 7px; }
VideoCell QToolButton#cellBtn:hover { color:#fff1c6; background:#3a2b17; border-color:#e6bd63; }
VideoCell QToolButton#cellPlay { color:#17130b; background:#dfb34e; border:2px solid #f4d88f; border-radius:32px; }
VideoCell[dropTarget="true"] { border:2px dashed #f0cf78; }
QWidget#cellAuthBox { background:rgba(10,11,12,240); }
QWidget#cellAuthBox QLabel { color:#e8dfcc; background:transparent; }
QLabel#cellAuthTitle { color:#e9c16a; font-weight:800; }
QPushButton#cellAuthBtn { color:#17130b; background:#d4aa56; border:1px solid #f0d28c; font-weight:800; }
QLabel#editionFooter { color:#e5c45a; }
)PSVMS");
}

// app/top_bar.h
// ======================================================================
class CameraManager;
class QLabel;
class QPushButton;

// Верхняя панель страницы: название раздела слева; справа счётчики «в сети / не в сети»
// (по щелчку открывается окно состояния камер) и кнопка облака Hik-Connect.
class TopBar : public QWidget {
    Q_OBJECT
public:
    explicit TopBar(CameraManager *mgr, QWidget *parent = nullptr);
    void setTitle(const QString &title);
    void setSettingsVisible(bool visible);

signals:
    void statusRequested();
    void settingsRequested();
    void fullscreenRequested();
    void cloudRequested();
    void simpleModeToggled(bool simple);

private:
    void updateCounts();
    void updateModeButton();
    void updateStatusVisibility();

    CameraManager *m_mgr;
    QLabel *m_title = nullptr;
    QLabel *m_online = nullptr;
    QLabel *m_offline = nullptr;
    QPushButton *m_status = nullptr;
    QPushButton *m_settings = nullptr;
    QPushButton *m_fullscreen = nullptr;
    QPushButton *m_cloud = nullptr;
    QPushButton *m_mode = nullptr;
    bool m_simple = false;
};

// ======================================================================
// app/ui_helpers.h
// ======================================================================
// Стандартные кнопки Qt всегда английские (OK/Cancel); подписываем их на выбранном языке.
inline void localizeButtons(QDialogButtonBox *box)
{
    if (QPushButton *ok = box->button(QDialogButtonBox::Ok))
        ok->setText(T("ОК"));
    if (QPushButton *cancel = box->button(QDialogButtonBox::Cancel))
        cancel->setText(T("Отмена"));
}

// Поле пароля со звёздочками и значком-глазком, который временно показывает пароль открытым текстом
// (пока зажата кнопка — отпустили, снова звёздочки). Возвращает контейнер для размещения в layout;
// сам QLineEdit отдаётся через edit (создавать его заранее не нужно).
inline QWidget *passwordFieldWithEye(QWidget *parent, QLineEdit **edit)
{
    auto *box = new QWidget(parent);
    auto *lay = new QHBoxLayout(box);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(4);

    auto *field = new QLineEdit(box);
    field->setEchoMode(QLineEdit::Password);
    auto *eye = new QPushButton(QStringLiteral("◉"), box);
    eye->setObjectName(QStringLiteral("eyeBtn"));
    eye->setCursor(Qt::PointingHandCursor);
    eye->setToolTip(T("Удерживайте, чтобы показать пароль"));
    eye->setFixedSize(42, 36);
    eye->setStyleSheet(QStringLiteral(
        "QPushButton#eyeBtn{color:#e7c16e;background:rgba(23,29,38,220);"
        "border:1px solid rgba(218,174,89,150);border-radius:10px;font-size:19px;font-weight:800;}"
        "QPushButton#eyeBtn:hover{color:#fff0bd;background:rgba(68,53,28,235);"
        "border-color:#f2d18a;}"));

    QObject::connect(eye, &QPushButton::pressed, field, [field, eye] {
        field->setEchoMode(QLineEdit::Normal);
        eye->setText(QStringLiteral("◎"));
        eye->setToolTip(T("Пароль виден"));
    });
    QObject::connect(eye, &QPushButton::released, field, [field, eye] {
        field->setEchoMode(QLineEdit::Password);
        eye->setText(QStringLiteral("◉"));
        eye->setToolTip(T("Удерживайте, чтобы показать пароль"));
    });

    auto *eyePulse = new QTimer(eye);
    eyePulse->setInterval(720);
    auto pulseState = std::make_shared<bool>(false);
    QObject::connect(eyePulse, &QTimer::timeout, eye, [eye, pulseState] {
        *pulseState = !*pulseState;
        eye->setStyleSheet(*pulseState
            ? QStringLiteral("QPushButton#eyeBtn{color:#fff0bd;background:rgba(48,55,68,235);border:1px solid #e7bd63;border-radius:10px;font-size:20px;font-weight:900;}")
            : QStringLiteral("QPushButton#eyeBtn{color:#e7c16e;background:rgba(23,29,38,220);border:1px solid rgba(218,174,89,150);border-radius:10px;font-size:19px;font-weight:800;}"));
    });
    auto updateEyeAnimation = [eyePulse] {
        if (Cfg::view().animations) eyePulse->start();
        else eyePulse->stop();
    };
    updateEyeAnimation();
    QObject::connect(Cfg::notifier(), &CfgNotifier::changed, eye, updateEyeAnimation);

    lay->addWidget(field, 1);
    lay->addWidget(eye);
    if (edit)
        *edit = field;
    return box;
}

// ======================================================================
// app/video_cell.h
// ======================================================================
class QAudioOutput;
class QCamera;
class QEnterEvent;
class QGraphicsOpacityEffect;
class QLabel;
class QLineEdit;
class QMediaCaptureSession;
class QMediaPlayer;
class QPropertyAnimation;
class QPushButton;
class QStackedLayout;
class QTimer;
class QToolButton;
class VideoSurface;

// Одна ячейка мозаики. Заголовок: название камеры (за него ячейку можно перетащить), звук, закрытие ✕.
// Остановленная ячейка показывает «▶». Двойной щелчок — на весь экран и обратно, колесо мыши —
// увеличение. Меню камеры по правой кнопке отключено: настройки — через «Устройства».
class VideoCell : public QFrame {
    Q_OBJECT
public:
    explicit VideoCell(QWidget *parent = nullptr);

    bool isEmpty() const { return m_cameraId.isEmpty(); }
    bool isStarted() const { return m_started; }
    bool isMuted() const { return m_muted; }
    QString cameraId() const { return m_cameraId; }
    QUrl url() const { return m_url; }
    VideoSurface *takeSurfaceForFullscreen();
    void restoreSurfaceFromFullscreen(VideoSurface *surface);

    void setCamera(const QString &id, const QString &title, const QUrl &url, bool autoplay,
                   bool showTime, int brightness, bool audio, bool fillCell = false);
    void setViewOptions(const QString &title, bool showTime, int brightness);
    void setFillCell(bool on);
    void startPlayback();
    void stopPlayback();                 // остановить показ: ячейка остаётся за камерой, показывается «▶»
    void clear(bool notify = true);
    void setSelected(bool on);
    void setDropTarget(bool on);
    void setMuted(bool muted);
    QImage snapshot() const;
    void resetZoom();
    void flash();                        // короткая вспышка после перестановки

signals:
    void clicked(VideoCell *cell);
    void doubleClicked(VideoCell *cell);
    void cameraDropped(VideoCell *cell, const QString &cameraId);
    void cellDropped(VideoCell *cell, quintptr sourceCell);
    void contextMenuRequested(VideoCell *cell, const QPoint &globalPos);
    void addRequested(VideoCell *cell);
    void assignmentChanged();
    // Камера отказала во входе; cell сам показал окошко с полями и подставил введённые данные —
    // здесь только просьба сохранить их в настройках камеры (новый m_url уже установлен).
    void credentialsEntered(VideoCell *cell, const QUrl &newUrl);

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;
    void enterEvent(QEnterEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragLeaveEvent(QDragLeaveEvent *e) override;
    void dropEvent(QDropEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void fitHintFont();   // размер шрифта подсказки «Перетащите камеру сюда» под размер ячейки

private:
    void restart();
    bool isLocalCapture() const;   // адрес вида qcam:... — локальная карта видеозахвата, не RTSP
    void updateIdlePage();
    void updateCellArtwork(); // векторная иконка камеры автоматически масштабируется и центрируется
    void startDrag();
    QPixmap dragPixmap() const;
    void showAuthPrompt();    // окошко логина/пароля прямо в ячейке — камера отказала во входе
    void hideAuthPrompt();
    void submitAuthPrompt();

    QString m_cameraId;
    QString m_titleText;
    QUrl m_url;
    bool m_started = false;
    bool m_muted = true;
    bool m_hover = false;
    QPoint m_dragStart;

    QLabel *m_title = nullptr;
    QToolButton *m_audioBtn = nullptr;
    QSlider *m_volumeSlider = nullptr;
    QToolButton *m_closeBtn = nullptr;
    QWidget *m_bar = nullptr;
    QWidget *m_idle = nullptr;
    QToolButton *m_plus = nullptr;
    QToolButton *m_play = nullptr;
    QLabel *m_logo = nullptr;
    QLabel *m_hint = nullptr;
    QGraphicsOpacityEffect *m_plusFx = nullptr;
    QPropertyAnimation *m_plusAnim = nullptr;
    QStackedLayout *m_stack = nullptr;
    VideoSurface *m_surface = nullptr;
    QMediaPlayer *m_player = nullptr;
    QAudioOutput *m_audioOut = nullptr;
    QCamera *m_camera = nullptr;                    // для аналоговых камер через карту видеозахвата
    QMediaCaptureSession *m_capture = nullptr;
    QTimer *m_retry = nullptr;
    QLabel *m_flash = nullptr;
    QGraphicsOpacityEffect *m_flashFx = nullptr;

    // окошко логина/пароля прямо в ячейке (создаётся лениво, только когда реально понадобилось)
    QWidget *m_authBox = nullptr;
    QLabel *m_authHost = nullptr;
    QLineEdit *m_authUser = nullptr;
    QLineEdit *m_authPass = nullptr;
    QPushButton *m_authBtn = nullptr;
    bool m_authShown = false;
};

// ======================================================================
// app/video_grid.h
// ======================================================================
class CameraManager;
class QGridLayout;
class QTimer;
class VideoCell;
struct CameraEntry;

struct GridLayoutDef {
    int count;   // число ячеек
    int cols;    // число столбцов
};

// Мозаика с раскладками от 1 до 64 ячеек. Ячейки можно перетаскивать друг на друга (камеры меняются
// местами), добавлять камеры из списка и кнопкой «+». Расположение камер сохраняется как шаблон
// автоматически; кроме того, можно хранить именованные шаблоны.
class VideoGrid : public QWidget {
    Q_OBJECT
public:
    explicit VideoGrid(CameraManager *mgr, QWidget *parent = nullptr);

    static QList<GridLayoutDef> layouts();
    static QString layoutLabel(int count);       // например, «3 × 2»
    int layoutCount() const { return m_count; }
    QStringList cameraIds() const;                // по ячейкам; пустая строка — свободная ячейка
    QString selectedCameraId() const;
    void flushLayout();                           // сохранить расположение камер сразу (при выходе)

    QStringList templateNames() const;
    void saveTemplate(const QString &name);
    void applyTemplateNamed(const QString &name);
    void removeTemplate(const QString &name);

public slots:
    void setLayoutCount(int n);
    void openCamera(const QString &cameraId);
    void clearAll();
    void startAllCells();
    void stopAllCells();
    void refreshCells();
    void restoreLayout();                         // последнее расположение камер
    void closeSelectedCell();                      // горячая клавиша Delete

signals:
    void message(const QString &text);
    void archiveRequested(const QString &cameraId);
    void addCameraRequested();
    void layoutCountChanged(int count);
    void selectionChanged(const QString &cameraId);   // выделенная ячейка сменилась (может быть пустой)

private:
    void ensureCells(int n);
    void applyLayout();
    void select(VideoCell *cell);
    void placeCamera(VideoCell *cell, const QString &cameraId, bool autoplay);
    QUrl urlFor(const CameraEntry &e, const VideoCell *cell) const;
    void applyTemplateData(int count, const QStringList &ids, bool autoplay);
    void saveSoon();
    void saveLayoutNow();
    void swapCells(VideoCell *a, VideoCell *b);
    void showCellMenu(VideoCell *cell, const QPoint &globalPos);
    void showAddMenu(VideoCell *cell, const QPoint &globalPos);
    void editCamera(const QString &cameraId);
    void saveSnapshot(VideoCell *cell);
    void openFullscreen(VideoCell *cell);

    CameraManager *m_mgr;
    QGridLayout *m_grid = nullptr;
    QList<VideoCell *> m_cells;
    VideoCell *m_selected = nullptr;
    QTimer *m_saveTimer = nullptr;
    int m_count = 4;
    bool m_restoring = false;
};

// ======================================================================
// app/video_surface.h
// ======================================================================
class QTimer;
class QVideoFrame;
class QVideoSink;

// Поверхность вывода видео. Сама рисует кадры (QVideoSink), поэтому умеет то, чего нет у QVideoWidget:
// ограничение частоты кадров для слабых компьютеров, увеличение колесом мыши, дату и время поверх
// картинки, яркость, снимок кадра.
class VideoSurface : public QWidget {
    Q_OBJECT
public:
    explicit VideoSurface(QWidget *parent = nullptr);

    QVideoSink *sink() const { return m_sink; }
    QImage currentFrame() const { return m_frame; }
    bool hasFrame() const { return !m_frame.isNull(); }

    void setShowTime(bool on);
    void setBrightness(int percent);          // от -50 до 50
    void setFillCell(bool on);                 // заполнить всю ячейку, обрезая края кадра
    void setStatusText(const QString &text);  // «Подключение…», «Нет сигнала»
    void setCameraRunning(bool running);      // фирменный индикатор запущенной камеры
    void resetZoom();
    void clearFrame();

    // качество связи: 0 — нет сигнала (красный), 1 — подключение (жёлтый), 2 — стабильно (зелёный)
    enum class LinkQuality { None, Connecting, Good };
    void setLinkQuality(LinkQuality q);

signals:
    void clicked();
    void doubleClicked();
    void contextMenuRequested(const QPoint &globalPos);

protected:
    void paintEvent(QPaintEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    void onFrame(const QVideoFrame &frame);
    QRectF targetRect() const;
    void updateInterval();

    QVideoSink *m_sink = nullptr;
    QImage m_frame;
    QRectF m_view{0.0, 0.0, 1.0, 1.0};   // видимая часть кадра в долях (0..1)
    QString m_status;
    bool m_showTime = true;
    bool m_fillCell = false;
    bool m_cameraRunning = false;
    int m_brightness = 0;
    qint64 m_lastFrameMs = 0;
    int m_minIntervalMs = 66;

    // измерение FPS: считаем кадры за последнюю секунду
    qint64 m_fpsWindowStartMs = 0;
    int m_fpsFrameCount = 0;
    int m_fps = 0;
    double m_decodedMbps = 0.0; // расчётная скорость декодированных кадров, не сетевой битрейт RTSP
    LinkQuality m_quality = LinkQuality::None;
    bool m_panning = false;
    QPoint m_panStart;
    QRectF m_panView;
    QTimer *m_clock = nullptr;
};

// Всплывающий предпросмотр камеры при наведении на её название в таблицах.
// Поток запускается только на время наведения и останавливается при уходе курсора.
class CameraHoverPreview final : public QObject {
public:
    CameraHoverPreview(QTableWidget *table, CameraManager *manager, QObject *parent = nullptr);
    ~CameraHoverPreview() override;
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
private:
    void schedule(const QString &cameraId, const QPoint &globalPos);
    void showPreview();
    void hidePreview();
    QPointer<QTableWidget> m_table;
    CameraManager *m_manager = nullptr;
    QTimer *m_delay = nullptr;
    QWidget *m_popup = nullptr;
    VideoSurface *m_video = nullptr;
    QMediaPlayer *m_player = nullptr;
    QAudioOutput *m_audio = nullptr;
    QLabel *m_info = nullptr;
    QString m_pendingId;
    QString m_currentId;
    QPoint m_globalPos;
};

// ======================================================================
// core/app_paths.h
// ======================================================================
// Все данные программы — настройки, архив, снимки экрана — по умолчанию хранятся рядом с самой
// программой (в подпапке data), а не в «Моих документах»: это и проще для переноса на другой
// компьютер (скопировать всю папку программы), и не раскидывает файлы по профилю Windows.
// Папка может быть недоступна для записи (например, программа установлена в Program Files) —
// тогда используется запасной вариант в %LOCALAPPDATA%\PSVMS.
namespace AppPaths {

QString dataDir();            // <папка программы>/data  (или запасной путь)
void initSettingsLocation();  // должен быть вызван первым, до любого QSettings

} // namespace AppPaths

// ======================================================================
// core/backup.h
// ======================================================================
// Резервная копия камер, настроек, шаблонов раскладки и расположения камер в окне.
// Пользователи и пароли входа в копию не попадают. ВНИМАНИЕ: адреса камер содержат пароли камер.
namespace Backup {

bool exportTo(const QString &path, QString *error);
bool importFrom(const QString &path, QString *error);   // после восстановления нужен перезапуск программы

} // namespace Backup

// ======================================================================
// core/icsee.h
// ======================================================================
// Камеры iCSee построены на чипах Xiongmai (XM, они же XMeye/Sofia).
// RTSP-адрес у них особый: логин и пароль передаются прямо в пути.
//   stream = 0 — основной поток, stream = 1 — дополнительный (меньше разрешение)
//   channel — номер канала (у камеры обычно 1, у регистраторов 1..N)
inline QString icseeRtspUrl(const QString &host, quint16 port, const QString &user,
                            const QString &pass, int channel, int stream)
{
    return QStringLiteral("rtsp://%1:%2/user=%3&password=%4&channel=%5&stream=%6.sdp?real_stream")
        .arg(host)
        .arg(port)
        .arg(QString::fromUtf8(QUrl::toPercentEncoding(user)),
             QString::fromUtf8(QUrl::toPercentEncoding(pass)))
        .arg(channel)
        .arg(stream);
}

// ======================================================================
// core/onvif_ptz.h
// ======================================================================
class QNetworkAccessManager;

// Параметры для управления поворотной камерой: адрес PTZ-сервиса и токен профиля —
// их возвращает OnvifResolver при добавлении камеры через импорт ONVIF (core/onvif.h).
struct PtzTarget {
    QString xAddr;
    QString token;
    QString user;
    QString password;

    bool valid() const { return !xAddr.isEmpty() && !token.isEmpty(); }
};

// Непрерывное движение, пока не придёт Stop: x/y/zoom — от -1.0 до 1.0 (скорость и направление).
// nam можно передавать временный — используется и забывается, ответ камеры не разбирается
// (команда либо выполнится, либо нет; повторная отправка безопасна).
void ptzContinuousMove(QNetworkAccessManager *nam, const PtzTarget &target, double x, double y, double zoom);
void ptzStop(QNetworkAccessManager *nam, const PtzTarget &target);

// ======================================================================
// core/rtsp_probe.h
// ======================================================================
// Подбор рабочего RTSP-адреса камеры. Новые и старые модели Hikvision / HiWatch,
// а также Polyvision и OEM-камеры (Dahua, Xiongmai) используют разные пути, поэтому
// программа по очереди отправляет RTSP DESCRIBE (с Basic/Digest-авторизацией) и ищет путь,
// на который камера отвечает «200 OK».
class RtspProbe : public QObject {
    Q_OBJECT
public:
    explicit RtspProbe(QObject *parent = nullptr);

    void start(const QString &host, quint16 port, const QString &user, const QString &pass);
    void cancel();

    static int templateCount();
    // Готовый адрес канала по шаблону; для подпотока — пустая строка, если шаблон его не знает
    static QString buildUrl(int templateIndex, const QString &host, quint16 port, const QString &user,
                            const QString &pass, int channel, bool sub);

signals:
    // error: "auth" — неверный логин или пароль, "none" — рабочий путь не найден или нет ответа
    void finished(bool ok, int templateIndex, const QString &error);

private:
    void tryNext();
    void request(int tmpl, const QByteArray &authHeader, bool authed);
    void onResponse(int tmpl, bool authed, int code, const QString &www);
    QByteArray buildAuthHeader(const QString &www) const;

    QString m_host;
    QString m_user;
    QString m_pass;
    QString m_lastUri;
    quint16 m_port = 554;
    int m_index = 0;
    int m_gen = 0;
};

// ======================================================================
// core/secure_store.h
// ======================================================================
// Защищённое хранение учётных данных камер. На Windows используется DPAPI
// (CryptProtectData/CryptUnprotectData) с привязкой к текущей учётной записи Windows.
// Старый формат enc:v1 поддерживается только для миграции; новые записи — enc:v2.
// DPAPI защищает секреты на диске, но не от вредоносного ПО, уже работающего
// от имени вошедшего пользователя. Для максимальной защиты дополнительно нужны
// BitLocker, отдельная непривилегированная учётная запись и подписанные релизы.
namespace SecureStore {

QString encrypt(const QString &plain);     // пустая строка остаётся пустой (не шифруем «ничего»)
QString decrypt(const QString &cipherB64);  // DPAPI для enc:v2
QString decryptLegacy(const QString &cipherB64); // только миграция enc:v1

} // namespace SecureStore

// ======================================================================
// core/stream_presets.h
// ======================================================================
struct StreamOption {
    QString label;   // например: «Подпоток (102)»
    QString url;
};

// Предлагает потоки камеры, подставляя номер потока в адрес по шаблонам производителей:
//   Hikvision / HiWatch:  .../Streaming/Channels/101, 102, 103  (основной, подпоток, третий)
//   Dahua и OEM:          ...&subtype=0, 1, 2
//   iCSee (Xiongmai):     ...&stream=0, 1
// Для остальных адресов (например, полученных по ONVIF) шаблона нет, список пуст.
QList<StreamOption> streamOptionsFor(const QString &url);

// ======================================================================
// core/sys_stats.h
// ======================================================================
// Загрузка процессора (%) и памяти (%, занято/всего в МБ). На Windows — через системные счётчики,
// на других платформах («сборка для разработки» вне Windows) значения недоступны (-1).
struct SysStats {
    double cpuPercent = -1.0;
    double memPercent = -1.0;
    qint64 memUsedMb = 0;
    qint64 memTotalMb = 0;
};

SysStats readSysStats();

// ======================================================================
// core/users.h
// ======================================================================
struct UserInfo {
    QString id;
    QString name;
    QString role;                 // "admin" или "operator"
    bool hasPassword = false;
    bool enabled = true;

    bool valid() const { return !id.isEmpty(); }
    bool isDeveloper() const { return role == QLatin1String("developer"); }
    bool isGuest() const { return role == QLatin1String("guest"); }
    bool isAdmin() const { return role == QLatin1String("admin") || isDeveloper(); }
};

// Учётные записи программы: Гость и Administrator без пароля;
// Разработчик обязан задать пароль при первом входе. В настройках хранятся
// только соль и PBKDF2-хэш пароля.
namespace Users {

inline constexpr int kMinPassword = 4;

QList<UserInfo> all();
UserInfo find(const QString &id);
void ensureDefault();                        // создаёт гостя, Administrator и Разработчика при необходимости
bool needsLoginDialog();                     // выбор пользователя показывается при каждом запуске

QString add(const QString &name, const QString &role, const QString &password, QString *error);
bool remove(const QString &id, QString *error);
bool setPassword(const QString &id, const QString &password, QString *error);   // пустой — без пароля
bool setRole(const QString &id, const QString &role, QString *error);   // "admin" или "operator"
bool setEnabled(const QString &id, bool enabled, QString *error);
bool verify(const QString &id, const QString &password);

// «Запомнить меня»: на компьютере хранится случайный ключ, а не пароль
QString issueRememberToken(const QString &id);
bool consumeRememberedLogin(QString *userId);
void forgetRemembered();

void setCurrent(const QString &id);
UserInfo current();
QString lastUserId();
void setLastUserId(const QString &id);

} // namespace Users

namespace SecurityAudit { void write(const QString &event, const QString &details = QString()); }

// ======================================================================
// app/analog_camera_dialog.cpp
// ======================================================================
AnalogCameraDialog::AnalogCameraDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(T("Аналоговая камера (карта видеозахвата)"));
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setMinimumWidth(520);

    auto *hint = new QLabel(
        T("Для камеры, подключённой через карту видеозахвата (USB или PCI). Устройство должно быть "
          "подключено и видно Windows. Запись в архив для таких камер не ведётся — доступен только "
          "живой просмотр."), this);
    hint->setWordWrap(true);
    hint->setStyleSheet(QStringLiteral("color:#8a8a8e;"));

    m_name = new QLineEdit(this);
    m_group = new QLineEdit(this);
    m_group->setPlaceholderText(T("необязательно"));
    m_device = new QComboBox(this);
    for (const QCameraDevice &d : QMediaDevices::videoInputs())
        m_device->addItem(d.description(), QString::fromLatin1(d.id().toBase64()));
    if (m_device->count() == 0)
        m_device->addItem(T("Устройства захвата не найдены"), QString());

    auto *form = new QFormLayout;
    form->addRow(T("Устройство"), m_device);
    form->addRow(T("Название"), m_name);
    form->addRow(T("Группа"), m_group);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    localizeButtons(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *lay = new QVBoxLayout(this);
    lay->addWidget(hint);
    lay->addLayout(form);
    lay->addWidget(buttons);
}

CameraEntry AnalogCameraDialog::entry() const
{
    CameraEntry e;
    e.name = m_name->text().trimmed();
    e.group = m_group->text().trimmed();
    e.url = QStringLiteral("qcam:") + m_device->currentData().toString();
    e.record = false;   // для локальных устройств захвата запись не поддерживается
    return e;
}

void AnalogCameraDialog::accept()
{
    if (m_device->currentData().toString().isEmpty()) {
        QMessageBox::warning(this, windowTitle(), T("Устройство захвата не выбрано или не найдено."));
        return;
    }
    if (m_name->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, windowTitle(), T("Укажите название камеры."));
        return;
    }
    QDialog::accept();
}

// ======================================================================
// app/camera_dialog.cpp
// ======================================================================
CameraDialog::CameraDialog(const CameraEntry &base, const QStringList &groups, QWidget *parent)
    : QDialog(parent), m_base(base)
{
    setWindowTitle(base.id.isEmpty() ? T("Добавить камеру") : T("Настройки камеры"));
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setMinimumWidth(640);

    m_name = new QLineEdit(base.name, this);
    m_group = new QComboBox(this);
    m_group->setEditable(true);
    m_group->addItems(groups);
    m_group->setCurrentText(base.group);

    m_url = new QLineEdit(base.url, this);
    m_url->setPlaceholderText(T("rtsp://логин:пароль@192.168.1.64:554/Streaming/Channels/101"));
    m_liveUrl = new QLineEdit(base.liveUrl, this);
    m_liveUrl->setPlaceholderText(T("необязательно: поток меньшего разрешения для мозаики"));
    m_urlPick = new QComboBox(this);
    m_livePick = new QComboBox(this);
    m_urlPick->setMinimumWidth(200);
    m_livePick->setMinimumWidth(200);

    m_quality = new QComboBox(this);
    m_quality->addItem(T("Авто: подпоток в мозаике, основной в раскладке 1 × 1"), 0);
    m_quality->addItem(T("Высокое: всегда основной поток"), 1);
    m_quality->addItem(T("Низкое: всегда подпоток (экономия ресурсов)"), 2);
    m_quality->setCurrentIndex(qBound(0, base.viewQuality, 2));

    m_brightness = new QSlider(Qt::Horizontal, this);
    m_brightness->setRange(-50, 50);
    m_brightness->setValue(base.brightness);
    m_brightLabel = new QLabel(QString::number(base.brightness), this);
    m_brightLabel->setMinimumWidth(36);
    connect(m_brightness, &QSlider::valueChanged, this, [this](int v) { m_brightLabel->setText(QString::number(v)); });
    auto *brightRow = new QWidget(this);
    auto *brightLay = new QHBoxLayout(brightRow);
    brightLay->setContentsMargins(0, 0, 0, 0);
    brightLay->addWidget(m_brightness, 1);
    brightLay->addWidget(m_brightLabel);

    m_record = new QCheckBox(T("Вести непрерывную запись в архив"), this);
    m_record->setChecked(base.record);
    m_showTime = new QCheckBox(T("Показывать дату и время на картинке"), this);
    m_showTime->setChecked(base.showTime);
    m_audio = new QCheckBox(T("Включать звук при показе"), this);
    m_audio->setChecked(base.audio);

    auto *form = new QFormLayout;
    form->addRow(T("Название"), m_name);
    form->addRow(T("Группа"), m_group);
    form->addRow(T("Поток записи"), withPicker(m_url, m_urlPick));
    form->addRow(T("Подпоток для мозаики"), withPicker(m_liveUrl, m_livePick));
    form->addRow(T("Качество картинки"), m_quality);
    form->addRow(T("Яркость"), brightRow);
    form->addRow(QString(), m_record);
    form->addRow(QString(), m_showTime);
    form->addRow(QString(), m_audio);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    localizeButtons(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // выбор потока из шаблонов производителя: в поле подставляется готовый адрес
    connect(m_url, &QLineEdit::textChanged, this, [this] { rebuildPickers(); });
    connect(m_urlPick, &QComboBox::activated, this, [this](int i) {
        const QString u = m_urlPick->itemData(i).toString();
        m_urlPick->setCurrentIndex(0);
        if (!u.isEmpty())
            m_url->setText(u);
    });
    connect(m_livePick, &QComboBox::activated, this, [this](int i) {
        const QString u = m_livePick->itemData(i).toString();
        m_livePick->setCurrentIndex(0);
        if (u == QLatin1String("-"))
            m_liveUrl->clear();
        else if (!u.isEmpty())
            m_liveUrl->setText(u);
    });
    rebuildPickers();

    auto *lay = new QVBoxLayout(this);
    lay->addLayout(form);
    lay->addWidget(buttons);
}

QWidget *CameraDialog::withPicker(QLineEdit *edit, QComboBox *combo)
{
    auto *w = new QWidget(this);
    auto *l = new QHBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(edit, 1);
    l->addWidget(combo);
    return w;
}

// Варианты потоков строятся по адресу записи (Hikvision/HiWatch, Dahua, iCSee)
void CameraDialog::rebuildPickers()
{
    const QList<StreamOption> opts = streamOptionsFor(m_url->text().trimmed());

    m_urlPick->clear();
    m_urlPick->addItem(T("Выбрать поток…"), QString());
    for (const StreamOption &o : opts)
        m_urlPick->addItem(o.label, o.url);
    m_urlPick->setEnabled(!opts.isEmpty());

    m_livePick->clear();
    m_livePick->addItem(T("Выбрать поток…"), QString());
    m_livePick->addItem(T("Как у записи (без подпотока)"), QStringLiteral("-"));
    for (const StreamOption &o : opts)
        m_livePick->addItem(o.label, o.url);

    const QString tip = opts.isEmpty() ? T("Для этого адреса шаблон потоков неизвестен: введите адрес вручную") : QString();
    m_urlPick->setToolTip(tip);
    m_livePick->setToolTip(tip);
}

CameraEntry CameraDialog::entry() const
{
    CameraEntry e = m_base;
    e.name = m_name->text().trimmed();
    e.group = m_group->currentText().trimmed();
    e.url = m_url->text().trimmed();
    e.liveUrl = m_liveUrl->text().trimmed();
    e.viewQuality = m_quality->currentData().toInt();
    e.brightness = m_brightness->value();
    e.record = m_record->isChecked();
    e.showTime = m_showTime->isChecked();
    e.audio = m_audio->isChecked();
    return e;
}

void CameraDialog::accept()
{
    const QUrl u(m_url->text().trimmed());
    if (m_name->text().trimmed().isEmpty() || !u.isValid() || u.scheme().isEmpty()) {
        QMessageBox::warning(this, windowTitle(), T("Укажите название и корректный адрес потока, например rtsp://…"));
        return;
    }
    QDialog::accept();
}

// ======================================================================
// app/camera_manager.cpp
// ======================================================================
namespace {

QString newCameraId()
{
    return QStringLiteral("cam") + QUuid::createUuid().toString(QUuid::Id128).left(8);
}

int probeIntervalMs()
{
    return Cfg::view().lowPower ? 60000 : 20000;
}

} // namespace

CameraManager::CameraManager(QObject *parent) : QObject(parent)
{
    m_storageRoot = Cfg::videoDir();
    QDir().mkpath(m_storageRoot);

    m_startTimer = new QTimer(this);
    m_startTimer->setInterval(120);
    connect(m_startTimer, &QTimer::timeout, this, &CameraManager::startNext);

    m_probeTimer = new QTimer(this);
    m_probeTimer->setInterval(probeIntervalMs());
    connect(m_probeTimer, &QTimer::timeout, this, &CameraManager::probeNow);
    connect(Cfg::notifier(), &CfgNotifier::changed, this, [this] { m_probeTimer->setInterval(probeIntervalMs()); });

    load();
    if (Cfg::autostartCameras()) {
        for (const CameraEntry &e : std::as_const(m_cameras))
            syncWorker(e);
    }
    m_probeTimer->start();
    QTimer::singleShot(800, this, &CameraManager::probeNow);
}

CameraManager::~CameraManager()
{
    shutdown();
}

const CameraEntry *CameraManager::find(const QString &id) const
{
    for (const CameraEntry &e : m_cameras)
        if (e.id == id)
            return &e;
    return nullptr;
}

CameraWorker::State CameraManager::stateOf(const QString &id) const
{
    return m_states.value(id, CameraWorker::State::Stopped);
}

bool CameraManager::isStarted(const QString &id) const
{
    return m_workers.contains(id) || m_startQueue.contains(id);
}

bool CameraManager::isOnline(const QString &id) const
{
    return m_online.value(id, false) || stateOf(id) == CameraWorker::State::Recording;
}

int CameraManager::onlineCount() const
{
    int n = 0;
    for (const CameraEntry &e : m_cameras)
        if (isOnline(e.id))
            ++n;
    return n;
}

int CameraManager::offlineCount() const
{
    return int(m_cameras.size()) - onlineCount();
}

int CameraManager::recordingCount() const
{
    int n = 0;
    for (const CameraEntry &e : m_cameras)
        if (stateOf(e.id) == CameraWorker::State::Recording)
            ++n;
    return n;
}

namespace {
static const QByteArray kPsvmsLicensePublicKey = QByteArray::fromHex(
    "0bfe4cb9c32f723e982c597f6df3aa72b982d9aa18fcf865850163580ed6e49e");

QString psvmsMachineFingerprint()
{
    QByteArray machine = QSysInfo::machineUniqueId();
    if (machine.isEmpty())
        machine = (QSysInfo::machineHostName() + QSysInfo::productType()
                   + QSysInfo::currentCpuArchitecture()).toUtf8();
    return QString::fromLatin1(QCryptographicHash::hash(machine, QCryptographicHash::Sha256)
                               .toHex().left(32)).toUpper();
}

bool psvmsVerifyLicense(const QByteArray &document, QJsonObject *payloadOut, QString *error)
{
    auto fail = [error](const QString &message) {
        if (error) *error = message;
        return false;
    };
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(document, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject())
        return fail(QStringLiteral("Файл лицензии имеет неверный формат JSON."));
    const QJsonObject root = doc.object();
    const QJsonObject payload = root.value(QStringLiteral("payload")).toObject();
    const QByteArray signature = QByteArray::fromBase64(
        root.value(QStringLiteral("signature")).toString().toLatin1());
    if (payload.isEmpty() || signature.size() != 64)
        return fail(QStringLiteral("В файле отсутствуют данные или цифровая подпись."));
    const QByteArray signedBytes = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    EVP_PKEY *key = EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, nullptr,
                                                 reinterpret_cast<const unsigned char *>(kPsvmsLicensePublicKey.constData()),
                                                 size_t(kPsvmsLicensePublicKey.size()));
    if (!key)
        return fail(QStringLiteral("Не удалось инициализировать проверку подписи."));
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    const bool initialized = ctx && EVP_DigestVerifyInit(ctx, nullptr, nullptr, nullptr, key) == 1;
    const bool verified = initialized &&
        EVP_DigestVerify(ctx,
            reinterpret_cast<const unsigned char *>(signature.constData()), size_t(signature.size()),
            reinterpret_cast<const unsigned char *>(signedBytes.constData()), size_t(signedBytes.size())) == 1;
    if (ctx) EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(key);
    if (!verified)
        return fail(QStringLiteral("Подпись лицензии недействительна. Файл изменён или выпущен не вашим генератором."));
    if (payload.value(QStringLiteral("schema")).toInt() != 1)
        return fail(QStringLiteral("Неподдерживаемая версия лицензии."));
    if (payload.value(QStringLiteral("machineId")).toString() != psvmsMachineFingerprint())
        return fail(QStringLiteral("Эта лицензия выпущена для другого компьютера."));
    const QString mode = payload.value(QStringLiteral("mode")).toString();
    if (mode == QStringLiteral("duration")) {
        if (payload.value(QStringLiteral("durationSeconds")).toVariant().toLongLong() <= 0)
            return fail(QStringLiteral("В лицензии указан неверный срок действия."));
    } else if (mode == QStringLiteral("fixed")) {
        const QDateTime expiry = QDateTime::fromString(
            payload.value(QStringLiteral("expiresAt")).toString(), Qt::ISODateWithMs);
        if (!expiry.isValid())
            return fail(QStringLiteral("В лицензии не указана корректная дата окончания."));
    } else {
        return fail(QStringLiteral("Неизвестный тип срока действия лицензии."));
    }
    if (payloadOut) *payloadOut = payload;
    if (error) error->clear();
    return true;
}

bool psvmsLicenseActive(QString *reason = nullptr, QDateTime *expiryOut = nullptr)
{
    QSettings settings;
    const QByteArray document = settings.value(QStringLiteral("license/document")).toByteArray();
    if (document.isEmpty()) {
        if (reason) *reason = QStringLiteral("Лицензия не активирована. Доступен демонстрационный режим.");
        return false;
    }
    QJsonObject payload;
    QString error;
    if (!psvmsVerifyLicense(document, &payload, &error)) {
        if (reason) *reason = error;
        return false;
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();
    const QString mode = payload.value(QStringLiteral("mode")).toString();
    QDateTime expiry;
    if (mode == QStringLiteral("duration")) {
        QDateTime activation = settings.value(QStringLiteral("license/firstActivationUtc")).toDateTime();
        if (!activation.isValid()) {
            activation = now;
            settings.setValue(QStringLiteral("license/firstActivationUtc"), activation);
            settings.setValue(QStringLiteral("license/lastSeenUtc"), activation);
        }
        const qint64 seconds = payload.value(QStringLiteral("durationSeconds")).toVariant().toLongLong();
        expiry = activation.addSecs(seconds);
    } else {
        expiry = QDateTime::fromString(payload.value(QStringLiteral("expiresAt")).toString(), Qt::ISODateWithMs);
        expiry = expiry.toUTC();
    }

    const QDateTime lastSeen = settings.value(QStringLiteral("license/lastSeenUtc")).toDateTime().toUTC();
    if (lastSeen.isValid() && now < lastSeen) {
        if (reason) *reason = QStringLiteral("Обнаружен перевод системных часов назад. Проверьте дату и время Windows.");
        return false;
    }
    if (now > expiry) {
        if (reason) *reason = QStringLiteral("Срок действия лицензии истёк: %1 UTC.")
            .arg(expiry.toString(QStringLiteral("dd.MM.yyyy HH:mm")));
        return false;
    }

    // Запоминаем последнее проверенное время, чтобы выявлять перевод часов назад
    // после перезапуска приложения. Запись ограничена одним разом в минуту.
    static QDateTime lastPersist;
    if (!lastSeen.isValid() || !lastPersist.isValid() || lastPersist.secsTo(now) >= 60) {
        settings.setValue(QStringLiteral("license/lastSeenUtc"), now);
        lastPersist = now;
    }
    if (expiryOut) *expiryOut = expiry;
    if (reason) *reason = QStringLiteral("Лицензия активна до %1 UTC.")
        .arg(expiry.toString(QStringLiteral("dd.MM.yyyy HH:mm")));
    return true;
}

bool psvmsImportLicense(const QString &path, QString *error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        if (error) *error = QStringLiteral("Не удалось открыть файл лицензии.");
        return false;
    }
    const QByteArray document = file.readAll();
    QJsonObject payload;
    if (!psvmsVerifyLicense(document, &payload, error))
        return false;
    QSettings settings;
    settings.setValue(QStringLiteral("license/document"), document);
    settings.remove(QStringLiteral("license/firstActivationUtc"));
    settings.remove(QStringLiteral("license/lastSeenUtc"));
    // Для duration первая активация начинается сразу после успешного импорта.
    if (payload.value(QStringLiteral("mode")).toString() == QStringLiteral("duration")) {
        const QDateTime now = QDateTime::currentDateTimeUtc();
        settings.setValue(QStringLiteral("license/firstActivationUtc"), now);
        settings.setValue(QStringLiteral("license/lastSeenUtc"), now);
    }
    if (error) error->clear();
    return true;
}

inline bool isProVersion()
{
    return psvmsLicenseActive();
}
}

QString CameraManager::add(CameraEntry e)
{
    if (!Users::current().isAdmin()) { SecurityAudit::write(QStringLiteral("permission_denied"), QStringLiteral("camera.add")); return QString(); }
    if (!isProVersion() && m_cameras.size() >= 10) {
        QMessageBox::information(nullptr, T("Демо-версия"),
            T("В демо-версии можно подключить не более 10 камер. Для снятия ограничения активируйте Pro-версию в настройках."));
        return QString();
    }
    e.id = newCameraId();
    m_cameras.append(e);
    save();
    SecurityAudit::write(QStringLiteral("camera_added"), e.id);
    syncWorker(e);
    emit camerasChanged();
    probeNow();
    return e.id;
}

void CameraManager::addMany(QList<CameraEntry> list)
{
    if (!Users::current().isAdmin()) { SecurityAudit::write(QStringLiteral("permission_denied"), QStringLiteral("camera.addMany")); return; }
    if (list.isEmpty())
        return;
    if (!isProVersion()) {
        const int available = qMax(0, 10 - m_cameras.size());
        if (available == 0) {
            QMessageBox::information(nullptr, T("Демо-версия"),
                T("Лимит демо-версии — 10 камер. Активируйте Pro-версию в настройках, чтобы добавить больше."));
            return;
        }
        if (list.size() > available) {
            list = list.mid(0, available);
            QMessageBox::information(nullptr, T("Демо-версия"),
                T("В демо-версии можно добавить ещё %1 камер. Остальные устройства не добавлены. Для полного доступа активируйте Pro-версию в настройках.").arg(available));
        }
    }
    for (CameraEntry &e : list) {
        e.id = newCameraId();
    }
    m_cameras.append(list);
    save();                                  // один раз на всю пачку
    SecurityAudit::write(QStringLiteral("camera_batch_added"), QStringLiteral("count=%1").arg(list.size()));
    for (const CameraEntry &e : std::as_const(list))
        syncWorker(e);
    emit camerasChanged();
    probeNow();
}

void CameraManager::enforceDemoLimit()
{
    if (isProVersion() || m_cameras.size() <= 10)
        return;

    for (int i = 10; i < m_cameras.size(); ++i) {
        const QString id = m_cameras.at(i).id;
        stopWorker(id);
        m_states.remove(id);
        m_online.remove(id);
    }
    m_cameras = m_cameras.mid(0, 10);
    save();
    emit camerasChanged();
    emit onlineChanged();
    QMessageBox::information(nullptr, T("Демо-версия"),
        T("В демо-режиме одновременно доступны не более 10 камер. Лишние камеры удалены из списка подключений."));
}

void CameraManager::update(const CameraEntry &e)
{
    if (!Users::current().isAdmin()) { SecurityAudit::write(QStringLiteral("permission_denied"), QStringLiteral("camera.update")); return; }
    for (CameraEntry &old : m_cameras) {
        if (old.id == e.id) {
            // рекордер перезапускаем, только если изменился адрес записи или её включение
            const bool restart = old.url != e.url || old.record != e.record;
            const bool wasStarted = isStarted(e.id);
            old = e;
            save();
            SecurityAudit::write(QStringLiteral("camera_updated"), e.id);
            if (restart && wasStarted)
                syncWorker(e);
            emit camerasChanged();
            return;
        }
    }
}

void CameraManager::setLiveUrl(const QString &id, const QString &url)
{
    for (CameraEntry &e : m_cameras) {
        if (e.id == id) {
            if (e.liveUrl == url)
                return;
            e.liveUrl = url;
            save();
            emit camerasChanged();
            return;
        }
    }
}

void CameraManager::remove(const QString &id)
{
    if (!Users::current().isAdmin()) { SecurityAudit::write(QStringLiteral("permission_denied"), QStringLiteral("camera.remove")); return; }
    SecurityAudit::write(QStringLiteral("camera_removed"), id);
    stopWorker(id);
    for (int i = 0; i < m_cameras.size(); ++i) {
        if (m_cameras[i].id == id) {
            m_cameras.removeAt(i);
            break;
        }
    }
    m_states.remove(id);
    m_online.remove(id);
    save();
    emit camerasChanged();
    emit onlineChanged();
}

void CameraManager::startCamera(const QString &id)
{
    if (isStarted(id))
        return;
    if (const CameraEntry *e = find(id))
        syncWorker(*e);
}

void CameraManager::stopCamera(const QString &id)
{
    stopWorker(id);
    m_states.insert(id, CameraWorker::State::Stopped);
    emit cameraStateChanged(id, CameraWorker::State::Stopped);
    emit onlineChanged();
}

void CameraManager::startAll()
{
    for (const CameraEntry &e : std::as_const(m_cameras))
        startCamera(e.id);
}

void CameraManager::stopAll()
{
    for (const CameraEntry &e : std::as_const(m_cameras))
        if (isStarted(e.id))
            stopCamera(e.id);
}

void CameraManager::setStorageRoot(const QString &path)
{
    if (path.isEmpty() || path == m_storageRoot)
        return;
    QSettings().setValue(QStringLiteral("storage/video"), path);
    m_storageRoot = path;
    QDir().mkpath(m_storageRoot);

    QStringList ids = m_workers.keys();
    ids += m_startQueue;
    ids.removeDuplicates();
    for (const QString &id : std::as_const(ids))
        if (const CameraEntry *e = find(id))
            syncWorker(*e);        // перезапуск с новой папкой
}

void CameraManager::shutdown()
{
    m_startQueue.clear();
    m_startTimer->stop();
    for (CameraWorker *w : std::as_const(m_workers))
        w->stop();
    for (CameraWorker *w : std::as_const(m_workers))
        w->wait();
    qDeleteAll(m_workers);
    m_workers.clear();
}

void CameraManager::load()
{
    QSettings s;
    const int n = s.beginReadArray(QStringLiteral("cameras"));
    bool needsCredentialMigration = false;
    for (int i = 0; i < n; ++i) {
        s.setArrayIndex(i);
        CameraEntry e;
        e.id = s.value(QStringLiteral("id")).toString();
        e.name = s.value(QStringLiteral("name")).toString();
        e.group = s.value(QStringLiteral("group")).toString();
        const QString storedUrl = s.value(QStringLiteral("url")).toString();
        if (!storedUrl.isEmpty() && !storedUrl.startsWith(QLatin1String("enc:v2:"))
            && !storedUrl.startsWith(QLatin1String("legacy:v1:")))
            needsCredentialMigration = true;
        e.url = storedUrl.startsWith(QLatin1String("enc:v2:"))
            ? SecureStore::decrypt(storedUrl.mid(7))
            : ((storedUrl.startsWith(QLatin1String("enc:v1:")) || storedUrl.startsWith(QLatin1String("legacy:v1:")))
                ? SecureStore::decryptLegacy(storedUrl.mid(storedUrl.startsWith(QLatin1String("legacy:v1:")) ? 10 : 7)) : storedUrl);
        const QString storedLiveUrl = s.value(QStringLiteral("liveUrl")).toString();
        if (!storedLiveUrl.isEmpty() && !storedLiveUrl.startsWith(QLatin1String("enc:v2:"))
            && !storedLiveUrl.startsWith(QLatin1String("legacy:v1:")))
            needsCredentialMigration = true;
        e.liveUrl = storedLiveUrl.startsWith(QLatin1String("enc:v2:"))
            ? SecureStore::decrypt(storedLiveUrl.mid(7))
            : ((storedLiveUrl.startsWith(QLatin1String("enc:v1:")) || storedLiveUrl.startsWith(QLatin1String("legacy:v1:")))
                ? SecureStore::decryptLegacy(storedLiveUrl.mid(storedLiveUrl.startsWith(QLatin1String("legacy:v1:")) ? 10 : 7)) : storedLiveUrl);
        e.record = s.value(QStringLiteral("record"), true).toBool();
        e.viewQuality = s.value(QStringLiteral("viewQuality"), 0).toInt();
        e.brightness = s.value(QStringLiteral("brightness"), 0).toInt();
        e.showTime = s.value(QStringLiteral("showTime"), true).toBool();
        e.audio = s.value(QStringLiteral("audio"), false).toBool();
        e.fillCell = s.value(QStringLiteral("fillCell"), false).toBool();
        e.ptzXAddr = s.value(QStringLiteral("ptzXAddr")).toString();
        e.ptzToken = s.value(QStringLiteral("ptzToken")).toString();
        e.ptzUser = s.value(QStringLiteral("ptzUser")).toString();
        const QString storedPtzPassword = s.value(QStringLiteral("ptzPassword")).toString();
        if (!storedPtzPassword.isEmpty() && !storedPtzPassword.startsWith(QLatin1String("enc:v2:"))
            && !storedPtzPassword.startsWith(QLatin1String("legacy:v1:")))
            needsCredentialMigration = true;
        e.ptzPassword = storedPtzPassword.startsWith(QLatin1String("enc:v2:"))
            ? SecureStore::decrypt(storedPtzPassword.mid(7))
            : ((storedPtzPassword.startsWith(QLatin1String("enc:v1:")) || storedPtzPassword.startsWith(QLatin1String("legacy:v1:")))
                ? SecureStore::decryptLegacy(storedPtzPassword.mid(storedPtzPassword.startsWith(QLatin1String("legacy:v1:")) ? 10 : 7)) : storedPtzPassword);
        if (!e.id.isEmpty())
            m_cameras.append(e);
    }
    s.endArray();

    QSettings migration;
    const QString migrationKey = QStringLiteral("migration/demoArchiveRecordingEnabled");
    if (!migration.value(migrationKey, false).toBool()) {
        for (CameraEntry &entry : m_cameras)
            entry.record = true;
        migration.setValue(migrationKey, true);
        save();
    } else if (needsCredentialMigration) {
        save(); // перезаписывает legacy-пароли в новом зашифрованном формате
    }

    if (!isProVersion() && m_cameras.size() > 10) {
        m_cameras = m_cameras.mid(0, 10);
        save();
        QTimer::singleShot(0, [] {
            QMessageBox::information(nullptr, T("Демо-версия"),
                T("В демо-режиме доступны первые 10 подключённых камер. Остальные устройства не загружены."));
        });
    }
}

void CameraManager::save() const
{
    QSettings s;
    s.remove(QStringLiteral("cameras"));
    s.beginWriteArray(QStringLiteral("cameras"), int(m_cameras.size()));
    for (int i = 0; i < m_cameras.size(); ++i) {
        s.setArrayIndex(i);
        const CameraEntry &e = m_cameras.at(i);
        s.setValue(QStringLiteral("id"), e.id);
        s.setValue(QStringLiteral("name"), e.name);
        s.setValue(QStringLiteral("group"), e.group);
        const QString encrypted_url = SecureStore::encrypt(e.url);
        s.setValue(QStringLiteral("url"), e.url.isEmpty() ? QString() :
                   (encrypted_url.startsWith(QLatin1String("legacy:v1:"))
                        ? encrypted_url : QStringLiteral("enc:v2:") + encrypted_url));
        const QString encrypted_liveUrl = SecureStore::encrypt(e.liveUrl);
        s.setValue(QStringLiteral("liveUrl"), e.liveUrl.isEmpty() ? QString() :
                   (encrypted_liveUrl.startsWith(QLatin1String("legacy:v1:"))
                        ? encrypted_liveUrl : QStringLiteral("enc:v2:") + encrypted_liveUrl));
        s.setValue(QStringLiteral("record"), e.record);
        s.setValue(QStringLiteral("viewQuality"), e.viewQuality);
        s.setValue(QStringLiteral("brightness"), e.brightness);
        s.setValue(QStringLiteral("showTime"), e.showTime);
        s.setValue(QStringLiteral("audio"), e.audio);
        s.setValue(QStringLiteral("fillCell"), e.fillCell);
        s.setValue(QStringLiteral("ptzXAddr"), e.ptzXAddr);
        s.setValue(QStringLiteral("ptzToken"), e.ptzToken);
        s.setValue(QStringLiteral("ptzUser"), e.ptzUser);
        const QString encrypted_ptzPassword = SecureStore::encrypt(e.ptzPassword);
        s.setValue(QStringLiteral("ptzPassword"), e.ptzPassword.isEmpty() ? QString() :
                   (encrypted_ptzPassword.startsWith(QLatin1String("legacy:v1:"))
                        ? encrypted_ptzPassword : QStringLiteral("enc:v2:") + encrypted_ptzPassword));
    }
    s.endArray();
    s.sync();
    if (s.status() != QSettings::NoError)
        writePsvmsLog(QStringLiteral("CameraManager: failed to persist camera settings securely."));
}

void CameraManager::stopWorker(const QString &id)
{
    m_startQueue.removeAll(id);
    if (CameraWorker *w = m_workers.take(id)) {
        w->stop();
        w->wait();
        delete w;
    }
}

void CameraManager::syncWorker(const CameraEntry &e)
{
    stopWorker(e.id);
    // Аналоговая камера через карту видеозахвата: это не RTSP-поток, рекордер на базе FFmpeg
    // открыть его не может — для таких камер доступен только живой просмотр
    if (!e.record || e.url.startsWith(QLatin1String("qcam:"))) {
        m_states.insert(e.id, CameraWorker::State::Stopped);
        emit cameraStateChanged(e.id, CameraWorker::State::Stopped);
        return;
    }
    // запуск не сразу, а через очередь
    m_states.insert(e.id, CameraWorker::State::Connecting);
    emit cameraStateChanged(e.id, CameraWorker::State::Connecting);
    m_startQueue.append(e.id);
    if (!m_startTimer->isActive())
        m_startTimer->start();
}

void CameraManager::startNext()
{
    if (m_startQueue.isEmpty()) {
        m_startTimer->stop();
        return;
    }
    const QString id = m_startQueue.takeFirst();
    const CameraEntry *e = find(id);
    if (!e || !e->record || m_workers.contains(id))
        return;

    CameraConfig cfg;
    cfg.id = e->id;
    cfg.name = e->name;
    cfg.url = e->url;
    cfg.storageRoot = m_storageRoot;
    cfg.dbPath = dbPath();

    auto *w = new CameraWorker(cfg, this);
    connect(w, &CameraWorker::stateChanged, this, [this](const QString &cid, CameraWorker::State st) {
        m_states.insert(cid, st);
        emit cameraStateChanged(cid, st);
        emit onlineChanged();
    });
    connect(w, &CameraWorker::errorOccurred, this, &CameraManager::cameraError);
    m_workers.insert(e->id, w);
    w->startRecording();
}

// ---- проверка доступности: короткое TCP-подключение к порту камеры ----

void CameraManager::probeNow()
{
    if (m_probing)
        return;
    m_probeQueue.clear();
    for (const CameraEntry &e : std::as_const(m_cameras))
        m_probeQueue.append(e.id);
    m_probeResults.clear();
    m_probing = true;
    pumpProbe();
}

void CameraManager::pumpProbe()
{
    while (m_probeActive < 16 && !m_probeQueue.isEmpty()) {
        const QString id = m_probeQueue.takeFirst();
        const CameraEntry *e = find(id);
        if (!e)
            continue;
        const QUrl u(e->url);
        const QString host = u.host();
        if (host.isEmpty()) {
            m_probeResults.insert(id, false);
            continue;
        }

        // Состояние «В сети» не должно зависеть только от RTSP-порта: часть камер
        // закрывает RTSP для неавторизованных клиентов, но отвечает на HTTP/HTTPS.
        QList<quint16> ports;
        auto appendPort = [&ports](int p) {
            if (p > 0 && p <= 65535 && !ports.contains(quint16(p)))
                ports.append(quint16(p));
        };
        appendPort(u.port(u.scheme() == QLatin1String("rtsps") ? 322 : 554));
        appendPort(554);
        appendPort(80);
        appendPort(8000);
        appendPort(443);

        ++m_probeActive;
        auto index = std::make_shared<int>(0);
        auto attempt = std::make_shared<std::function<void()>>();
        *attempt = [this, id, host, ports, index, attempt]() {
            if (*index >= ports.size()) {
                m_probeResults.insert(id, false);
                --m_probeActive;
                *attempt = {};
                pumpProbe();
                return;
            }
            const quint16 port = ports.at((*index)++);
            auto *sock = new QTcpSocket(this);
            auto socketDone = std::make_shared<bool>(false);
            auto finish = [this, sock, id, socketDone, attempt](bool ok) {
                if (*socketDone)
                    return;
                *socketDone = true;
                sock->abort();
                sock->deleteLater();
                if (ok) {
                    m_probeResults.insert(id, true);
                    --m_probeActive;
                    *attempt = {};
                    pumpProbe();
                } else {
                    (*attempt)();
                }
            };
            connect(sock, &QTcpSocket::connected, this, [finish] { finish(true); });
            connect(sock, &QAbstractSocket::errorOccurred, this,
                    [finish](QAbstractSocket::SocketError) { finish(false); });
            QTimer::singleShot(1200, sock, [finish] { finish(false); });
            sock->connectToHost(host, port);
        };
        (*attempt)();
    }

    if (m_probing && m_probeQueue.isEmpty() && m_probeActive == 0) {
        m_probing = false;
        m_online = m_probeResults;
        emit onlineChanged();
    }
}

// ======================================================================
// app/cloud_dialog.cpp
// ======================================================================
CloudDialog::CloudDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(T("Облако Hik-Connect"));
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setMinimumWidth(520);

    QSettings s;

    auto *info = new QLabel(
        T("Hik-Connect — облачный сервис Hikvision. Подключение к нему из сторонних программ "
          "возможно только через официальный SDK и ключи разработчика Hikvision (Open Platform), "
          "поэтому в этой версии получение камер из облака не работает."), this);
    info->setWordWrap(true);
    info->setStyleSheet(QStringLiteral("color:#e8a63a;"));

    m_region = new QComboBox(this);
    m_region->addItems({T("Россия и СНГ"), T("Европа"), T("Азия"), T("Америка"), T("Другой регион")});
    m_region->setCurrentIndex(qBound(0, s.value(QStringLiteral("cloud/region"), 0).toInt(), 4));
    m_account = new QLineEdit(s.value(QStringLiteral("cloud/account")).toString(), this);
    m_account->setPlaceholderText(T("e-mail или телефон"));
    m_password = new QLineEdit(this);
    m_password->setEchoMode(QLineEdit::Password);

    m_status = new QLabel(T("Статус: не подключено"), this);

    auto *form = new QFormLayout;
    form->addRow(T("Регион"), m_region);
    form->addRow(T("Аккаунт"), m_account);
    form->addRow(T("Пароль"), m_password);

    auto *connectBtn = new QPushButton(T("Подключиться"), this);
    auto *buttons = new QDialogButtonBox(this);
    buttons->addButton(connectBtn, QDialogButtonBox::ActionRole);
    buttons->addButton(QDialogButtonBox::Close)->setText(T("Закрыть"));
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons->button(QDialogButtonBox::Close), &QPushButton::clicked, this, &QDialog::accept);

    connect(connectBtn, &QPushButton::clicked, this, [this] {
        QSettings st;
        st.setValue(QStringLiteral("cloud/region"), m_region->currentIndex());
        st.setValue(QStringLiteral("cloud/account"), m_account->text().trimmed());   // пароль не сохраняем
        m_status->setText(T("Статус: недоступно в этой версии (нужны ключи Hikvision Open Platform). "
                            "Добавляйте камеры по IP в локальной сети."));
        m_status->setWordWrap(true);
    });

    auto *lay = new QVBoxLayout(this);
    lay->addWidget(info);
    lay->addLayout(form);
    lay->addWidget(m_status);
    lay->addWidget(buttons);
}

// ======================================================================
// app/credentials_dialog.cpp
// ======================================================================
bool askCredentials(QWidget *parent, const QString &address, QString *user, QString *password)
{
    QDialog dlg(parent);
    dlg.setWindowTitle(T("Нужен пароль"));
    dlg.setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    dlg.setMinimumWidth(400);

    auto *hint = new QLabel(T("Устройство %1 требует логин и пароль.\nВведите данные для входа.").arg(address), &dlg);
    hint->setWordWrap(true);
    auto *u = new QLineEdit(user ? *user : QString(), &dlg);
    auto *p = new QLineEdit(password ? *password : QString(), &dlg);
    p->setEchoMode(QLineEdit::Password);
    p->setPlaceholderText(T("Пароль устройства"));

    auto *form = new QFormLayout;
    form->addRow(T("Логин"), u);
    form->addRow(T("Пароль"), p);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    localizeButtons(buttons);
    buttons->button(QDialogButtonBox::Cancel)->setText(T("Пропустить"));
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    auto *lay = new QVBoxLayout(&dlg);
    lay->addWidget(hint);
    lay->addLayout(form);
    lay->addWidget(buttons);

    p->setFocus();
    if (dlg.exec() != QDialog::Accepted)
        return false;
    if (user)
        *user = u->text().trimmed();
    if (password)
        *password = p->text();
    return true;
}

// ======================================================================
// app/devices_page.cpp
// ======================================================================
namespace {

enum DevColumn { DevColName, DevColGroup, DevColAddress, DevColLink, DevColRecord, DevColCount };

QStringList cameraGroups(const CameraManager *mgr)
{
    QStringList g;
    for (const CameraEntry &e : mgr->cameras())
        if (!e.group.isEmpty() && !g.contains(e.group))
            g << e.group;
    return g;
}

} // namespace

DevicesPage::DevicesPage(CameraManager *mgr, QWidget *parent) : QWidget(parent), m_mgr(mgr)
{
    // ---- «Добавить» с выпадающим меню ----
    m_addBtn = new QToolButton(this);
    m_addBtn->setText(T("Добавить"));
    m_addBtn->setPopupMode(QToolButton::InstantPopup);
    auto *menu = new QMenu(m_addBtn);
    menu->setToolTipsVisible(true);
    QMenu *guided = menu->addMenu(T("Быстрое добавление"));
    guided->setIcon(makeCellIcon(QStringLiteral("plus")));
    connect(guided->addAction(T("По RTSP-адресу…")), &QAction::triggered, this, [this] {
        RtspQuickDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted)
            m_mgr->add(dlg.entry());
    });
    connect(guided->addAction(T("Камеру вручную (с подробными настройками)…")), &QAction::triggered, this, [this] { addCameraManually(); });
    connect(guided->addAction(T("Аналоговую камеру (карта видеозахвата)…")), &QAction::triggered, this, [this] {
        AnalogCameraDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted)
            m_mgr->add(dlg.entry());
    });
    menu->addSeparator();
    QMenu *network = menu->addMenu(T("Обнаружение в локальной сети"));
    network->setToolTip(T("Поиск камер и регистраторов в вашей сети"));
    connect(network->addAction(T("Добавить по IP / производителю…")), &QAction::triggered, this, [this] {
        HikIpDialog dlg(m_mgr, this);
        dlg.exec();
    });
    connect(network->addAction(T("Автоматический поиск (SADP + ONVIF)…")), &QAction::triggered, this, [this] {
        LanSearchDialog dlg(m_mgr, this);
        dlg.exec();
    });
    menu->addSeparator();
    QMenu *importMenu = menu->addMenu(T("Импорт и совместимость"));
    connect(importMenu->addAction(T("Импорт по ONVIF…")), &QAction::triggered, this, [this] {
        OnvifImportDialog dlg(m_mgr, this);
        dlg.exec();
    });
    connect(importMenu->addAction(T("Камеры iCSee…")), &QAction::triggered, this, [this] {
        IcseeImportDialog dlg(m_mgr, this);
        dlg.exec();
    });
    m_addBtn->setMenu(menu);

    m_edit = new QPushButton(T("Изменить"), this);
    auto *renameGroupBtn = new QPushButton(T("Переименовать группу…"), this);
    m_remove = new QPushButton(T("Удалить выбранное"), this);
    m_removeAll = new QPushButton(T("Удалить все камеры"), this);
    m_removeAll->setToolTip(T("Удалить все камеры и каналы регистраторов из списка PSVMS"));
    auto *start = new QPushButton(T("Запустить запись"), this);
    auto *stop = new QPushButton(T("Остановить запись"), this);
    auto *deviceFilter = new QLineEdit(this);
    deviceFilter->setPlaceholderText(T("Поиск по названию, IP или группе…"));
    deviceFilter->setClearButtonEnabled(true);
    deviceFilter->setMinimumWidth(210);
    start->setToolTip(T("Для выделенных камер; если ничего не выделено — для всех"));
    stop->setToolTip(T("Для выделенных камер; если ничего не выделено — для всех"));

    auto *top = new QHBoxLayout;
    m_addBtn->setText(T("＋  Добавить устройство"));
    m_addBtn->setToolTip(T("Добавить IP-камеру, регистратор или устройство видеозахвата"));
    top->addWidget(m_addBtn);
    top->addWidget(m_edit);
    top->addWidget(renameGroupBtn);
    top->addWidget(m_remove);
    top->addWidget(m_removeAll);
    top->addWidget(deviceFilter);
    top->addStretch(1);
    top->addWidget(start);
    top->addWidget(stop);

    m_table = new QTableWidget(0, DevColCount, this);
    m_table->setHorizontalHeaderLabels({T("Название"), T("Группа"), T("Адрес"), T("Связь"), T("Запись")});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(DevColAddress, QHeaderView::Stretch);
    m_table->setColumnWidth(DevColName, 240);
    m_table->setColumnWidth(DevColGroup, 200);
    m_table->setColumnWidth(DevColLink, 110);
    m_table->setColumnWidth(DevColRecord, 220);
    m_table->setMouseTracking(true);
    m_table->viewport()->setMouseTracking(true);
    new CameraHoverPreview(m_table, m_mgr, m_table);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(16, 12, 16, 12);
    lay->addLayout(top);
    lay->addWidget(m_table, 1);

    connect(m_edit, &QPushButton::clicked, this, &DevicesPage::editCamera);
    connect(m_remove, &QPushButton::clicked, this, &DevicesPage::removeCamera);
    connect(m_removeAll, &QPushButton::clicked, this, [this] {
        if (!Users::current().isAdmin())
            return;
        const QStringList ids = [&] {
            QStringList all;
            for (const CameraEntry &entry : m_mgr->cameras())
                all << entry.id;
            return all;
        }();
        if (ids.isEmpty())
            return;
        QMessageBox box(QMessageBox::Warning, T("Удалить все камеры"),
            T("Удалить ВСЕ камеры и каналы регистраторов (%1) из PSVMS?\nВсе потоки и запись будут остановлены. Уже сохранённые файлы архива останутся на диске. Это действие нельзя отменить.").arg(ids.size()),
            QMessageBox::NoButton, this);
        QPushButton *removeButton = box.addButton(T("Удалить все"), QMessageBox::DestructiveRole);
        box.addButton(T("Отмена"), QMessageBox::RejectRole);
        box.exec();
        if (box.clickedButton() != removeButton)
            return;
        for (const QString &id : ids)
            m_mgr->remove(id);
    });
    connect(renameGroupBtn, &QPushButton::clicked, this, [this] {
        const QStringList groups = cameraGroups(m_mgr);
        if (groups.isEmpty()) {
            QMessageBox::information(this, T("Переименовать группу"), T("Групп пока нет: укажите группу хотя бы у одной камеры."));
            return;
        }
        bool ok = false;
        const QString oldName = QInputDialog::getItem(this, T("Переименовать группу"), T("Группа:"), groups, 0, false, &ok);
        if (!ok || oldName.isEmpty())
            return;
        QString newName;
        {
            QInputDialog dlg(this);
            dlg.setWindowTitle(T("Переименовать группу"));
            dlg.setLabelText(T("Новое название группы «%1»:").arg(oldName));
            dlg.setTextValue(oldName);
            dlg.setOkButtonText(T("ОК"));
            dlg.setCancelButtonText(T("Отмена"));
            if (dlg.exec() != QDialog::Accepted)
                return;
            newName = dlg.textValue().trimmed();
        }
        if (newName.isEmpty() || newName == oldName)
            return;
        for (const CameraEntry &e : m_mgr->cameras()) {
            if (e.group == oldName) {
                CameraEntry c = e;
                c.group = newName;
                m_mgr->update(c);
            }
        }
    });
    connect(start, &QPushButton::clicked, this, &DevicesPage::startSelected);
    connect(stop, &QPushButton::clicked, this, &DevicesPage::stopSelected);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, [this] { editCamera(); });
    connect(deviceFilter, &QLineEdit::textChanged, this, [this](const QString &query) {
        const QString needle = query.trimmed();
        for (int row = 0; row < m_table->rowCount(); ++row) {
            bool match = needle.isEmpty();
            for (int col = 0; !match && col < m_table->columnCount(); ++col) {
                const QTableWidgetItem *item = m_table->item(row, col);
                match = item && item->text().contains(needle, Qt::CaseInsensitive);
            }
            m_table->setRowHidden(row, !match);
        }
    });
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &DevicesPage::updateButtons);
    connect(m_mgr, &CameraManager::camerasChanged, this, &DevicesPage::rebuild);
    connect(m_mgr, &CameraManager::cameraStateChanged, this, &DevicesPage::refreshStates);
    connect(m_mgr, &CameraManager::onlineChanged, this, &DevicesPage::refreshStates);

    rebuild();
}

QString DevicesPage::currentId() const
{
    const int row = m_table->currentRow();
    if (row < 0)
        return QString();
    const QTableWidgetItem *it = m_table->item(row, DevColName);
    return it ? it->data(Qt::UserRole).toString() : QString();
}

QStringList DevicesPage::selectedIds() const
{
    QStringList ids;
    const QModelIndexList rows = m_table->selectionModel()->selectedRows(DevColName);
    for (const QModelIndex &idx : rows)
        ids << m_table->item(idx.row(), DevColName)->data(Qt::UserRole).toString();
    return ids;
}

void DevicesPage::updateButtons()
{
    const bool has = !selectedIds().isEmpty();
    m_edit->setEnabled(has);
    m_remove->setEnabled(has && Users::current().isAdmin());
    m_removeAll->setEnabled(!m_mgr->cameras().isEmpty() && Users::current().isAdmin());
}

void DevicesPage::rebuild()
{
    const QStringList keep = selectedIds();
    m_table->setUpdatesEnabled(false);
    m_table->setRowCount(0);

    for (const CameraEntry &e : m_mgr->cameras()) {
        const int row = m_table->rowCount();
        m_table->insertRow(row);

        auto *name = new QTableWidgetItem(e.name);
        name->setData(Qt::UserRole, e.id);
        m_table->setItem(row, DevColName, name);
        m_table->setItem(row, DevColGroup, new QTableWidgetItem(e.group));

        // логин и пароль на экране не показываем (у iCSee пароль передаётся в самом пути)
        QString addr = QUrl(e.url).toString(QUrl::RemoveUserInfo);
        addr.replace(QRegularExpression(QStringLiteral("password=[^&]*")), QStringLiteral("password=***"));
        m_table->setItem(row, DevColAddress, new QTableWidgetItem(addr));
        m_table->setItem(row, DevColLink, new QTableWidgetItem());
        m_table->setItem(row, DevColRecord, new QTableWidgetItem());
        if (keep.contains(e.id))
            m_table->selectRow(row);
    }
    m_table->setUpdatesEnabled(true);
    refreshStates();
    updateButtons();
}

void DevicesPage::refreshStates()
{
    for (int row = 0; row < m_table->rowCount(); ++row) {
        const QString id = m_table->item(row, DevColName)->data(Qt::UserRole).toString();
        const CameraEntry *e = m_mgr->find(id);
        if (!e)
            continue;
        const bool online = m_mgr->isOnline(id);
        QTableWidgetItem *link = m_table->item(row, DevColLink);
        link->setText(online ? T("В сети") : T("Не в сети"));
        link->setForeground(QBrush(QColor(online ? "#3fb950" : "#e8833a")));

        QString rec;
        if (!e->record)
            rec = T("Запись выключена");
        else if (!m_mgr->isStarted(id))
            rec = T("Не запущена");
        else
            rec = stateText(m_mgr->stateOf(id));
        m_table->item(row, DevColRecord)->setText(rec);
    }
}

void DevicesPage::addCameraManually()
{
    CameraEntry base;
    CameraDialog dlg(base, cameraGroups(m_mgr), this);
    if (dlg.exec() == QDialog::Accepted)
        m_mgr->add(dlg.entry());
}

void DevicesPage::editCamera()
{
    const CameraEntry *cur = m_mgr->find(currentId());
    if (!cur)
        return;
    CameraDialog dlg(*cur, cameraGroups(m_mgr), this);
    if (dlg.exec() == QDialog::Accepted)
        m_mgr->update(dlg.entry());
}

void DevicesPage::removeCamera()
{
    const QStringList ids = selectedIds();
    if (ids.isEmpty())
        return;
    // кнопки подписываем сами: стандартные Yes/No у Qt всегда английские
    const QString text = ids.size() == 1
        ? T("Удалить камеру «%1»?\nУже записанные файлы архива останутся на диске.").arg(m_mgr->find(ids.first()) ? m_mgr->find(ids.first())->name : QString())
        : T("Удалить выбранные камеры (%1)?\nУже записанные файлы архива останутся на диске.").arg(ids.size());
    QMessageBox box(QMessageBox::Question, T("Удаление камеры"), text, QMessageBox::NoButton, this);
    QPushButton *yes = box.addButton(T("Удалить"), QMessageBox::AcceptRole);
    box.addButton(T("Отмена"), QMessageBox::RejectRole);
    box.exec();
    if (box.clickedButton() != yes)
        return;
    for (const QString &id : ids)
        m_mgr->remove(id);
}

void DevicesPage::startSelected()
{
    const QStringList ids = selectedIds();
    if (ids.isEmpty()) {
        m_mgr->startAll();
        return;
    }
    for (const QString &id : ids)
        m_mgr->startCamera(id);
}

void DevicesPage::stopSelected()
{
    const QStringList ids = selectedIds();
    if (ids.isEmpty()) {
        m_mgr->stopAll();
        return;
    }
    for (const QString &id : ids)
        m_mgr->stopCamera(id);
}

// ======================================================================
// app/events_page.cpp
// ======================================================================
EventsPage::EventsPage(QWidget *parent) : QWidget(parent)
{
    auto *clear = new QPushButton(T("Очистить"), this);
    auto *top = new QHBoxLayout;
    top->addWidget(clear);
    top->addStretch(1);

    m_table = new QTableWidget(0, 3, this);
    m_table->setHorizontalHeaderLabels(
        {T("Время"), T("Камера"), T("Событие")});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->setColumnWidth(0, 170);
    m_table->setColumnWidth(1, 220);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(12, 12, 12, 12);
    lay->addLayout(top);
    lay->addWidget(m_table, 1);

    connect(clear, &QPushButton::clicked, this, [this] { m_table->setRowCount(0); });
}

void EventsPage::addEvent(const QString &camera, const QString &text)
{
    m_table->insertRow(0);   // новые события сверху
    m_table->setItem(0, 0, new QTableWidgetItem(
        QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))));
    m_table->setItem(0, 1, new QTableWidgetItem(camera));
    m_table->setItem(0, 2, new QTableWidgetItem(text));

    while (m_table->rowCount() > 1000)   // не даём журналу расти бесконечно
        m_table->removeRow(m_table->rowCount() - 1);
}

// ======================================================================
// app/fullscreen_view.cpp
// ======================================================================
FullscreenView::FullscreenView(VideoCell *cell, QWidget *parent)
    : QWidget(parent, Qt::Window), m_cell(cell)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(cell ? cell->windowTitle() : QStringLiteral("PSVMS"));
    setStyleSheet(QStringLiteral("background: #000000;"));

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    if (cell) {
        m_surface = cell->takeSurfaceForFullscreen();
        if (m_surface)
            lay->addWidget(m_surface);
        connect(cell, &QObject::destroyed, this, [this] { m_cell = nullptr; });
    }

    m_hint = new QLabel(T("Esc или двойной щелчок — выйти"), this);
    m_hint->setStyleSheet(QStringLiteral("color:#f2f2f2; background: rgba(0,0,0,160); padding: 8px 14px; border-radius: 6px;"));
    m_hint->adjustSize();
    m_hint->move(24, 20);
    m_hint->raise();
    QTimer::singleShot(3500, m_hint, &QWidget::hide);

    m_close = new QToolButton(this);
    m_close->setText(QStringLiteral("✕"));
    m_close->setToolTip(T("Закрыть"));
    m_close->setStyleSheet(QStringLiteral(
        "QToolButton { color:#ffffff; background: rgba(0,0,0,160); border: none; border-radius: 6px; padding: 8px 14px; font-size: 16px; }"
        "QToolButton:hover { background: #a02b2b; }"));
    m_close->adjustSize();
    m_close->raise();
    connect(m_close, &QToolButton::clicked, this, &QWidget::close);

    // Esc должно срабатывать даже когда фокус находится на видеоповерхности.
    auto *escapeKey = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    escapeKey->setContext(Qt::WidgetWithChildrenShortcut);
    connect(escapeKey, &QShortcut::activated, this, &QWidget::close);

    if (m_surface) {
        connect(m_surface, &VideoSurface::doubleClicked, this, &QWidget::close);
        // Повторный двойной щелчок по изображению всегда возвращает видеопоток в мозаику.
        m_surface->setFocusPolicy(Qt::StrongFocus);
    }
}

void FullscreenView::openOnScreenOf(QWidget *w)
{
    if (w && w->screen())
        setGeometry(w->screen()->geometry());   // тот экран, где открыта программа
    showFullScreen();
    activateWindow();
}

void FullscreenView::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
        close();
    else
        QWidget::keyPressEvent(e);
}

void FullscreenView::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    m_close->move(width() - m_close->width() - 24, 20);
}

void FullscreenView::closeEvent(QCloseEvent *e)
{
    if (m_cell && m_surface) {
        if (layout())
            layout()->removeWidget(m_surface);
        m_cell->restoreSurfaceFromFullscreen(m_surface);
        m_surface = nullptr;
    }
    QWidget::closeEvent(e);
}

// ======================================================================
// app/hik_ip_dialog.cpp
// ======================================================================
HikIpDialog::HikIpDialog(CameraManager *mgr, QWidget *parent) : QDialog(parent), m_mgr(mgr)
{
    setWindowTitle(T("Добавить по IP: Hikvision, HiWatch, Dahua"));
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setMinimumWidth(560);

    m_adder = new DeviceAdder(this);

    m_ip = new QLineEdit(this);
    m_ip->setPlaceholderText(QStringLiteral("192.168.1.64"));
    m_httpPort = new QSpinBox(this);
    m_httpPort->setRange(1, 65535);
    m_httpPort->setValue(80);
    m_rtspPort = new QSpinBox(this);
    m_rtspPort->setRange(1, 65535);
    m_rtspPort->setValue(554);
    m_user = new QLineEdit(QStringLiteral("admin"), this);
    m_pass = new QLineEdit(this);
    m_pass->setEchoMode(QLineEdit::Password);
    m_channels = new QSpinBox(this);
    m_channels->setRange(0, 128);
    m_channels->setSpecialValueText(T("авто"));
    m_channels->setToolTip(T("Число каналов регистратора, если автоопределение не сработало"));
    m_group = new QLineEdit(this);
    m_group->setPlaceholderText(T("для регистратора — по умолчанию его имя"));
    m_name = new QLineEdit(this);
    m_name->setPlaceholderText(T("необязательно"));

    auto *portRow = new QHBoxLayout;
    portRow->addWidget(new QLabel(T("HTTP-порт"), this));
    portRow->addWidget(m_httpPort);
    portRow->addSpacing(12);
    portRow->addWidget(new QLabel(T("RTSP-порт"), this));
    portRow->addWidget(m_rtspPort);
    portRow->addStretch(1);

    auto *form = new QFormLayout;
    form->addRow(T("IP-адрес"), m_ip);
    form->addRow(T("Порты"), portRow);
    form->addRow(T("Логин"), m_user);
    form->addRow(T("Пароль"), m_pass);
    form->addRow(T("Каналов в регистраторе"), m_channels);
    form->addRow(T("Название"), m_name);
    form->addRow(T("Группа"), m_group);

    auto *hint = new QLabel(T("Работает с новыми и старыми моделями камер и регистраторов Hikvision и HiWatch. "
                              "Регистраторы Dahua и совместимые определяются автоматически по RTSP-адресу, "
                              "без списка моделей и каналов — укажите число каналов регистратора вручную, если оно известно. "
                              "Если устройство потребует другой логин или пароль, программа спросит."), this);
    hint->setWordWrap(true);
    hint->setStyleSheet(QStringLiteral("color:#8a8a8e;"));

    m_progress = new QProgressBar(this);
    m_progress->setRange(0, 1);
    m_progress->setValue(0);
    m_status = new QLabel(this);
    m_status->setWordWrap(true);
    m_btn = new QPushButton(T("Проверить и добавить"), this);
    auto *close = new QPushButton(T("Закрыть"), this);
    auto *bottom = new QHBoxLayout;
    bottom->addWidget(m_progress, 1);
    bottom->addWidget(m_btn);
    bottom->addWidget(close);

    auto *root = new QVBoxLayout(this);
    root->addWidget(hint);
    root->addLayout(form);
    root->addWidget(m_status);
    root->addLayout(bottom);

    connect(close, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_btn, &QPushButton::clicked, this, [this] {
        const QString ip = m_ip->text().trimmed();
        if (ip.isEmpty()) {
            QMessageBox::warning(this, windowTitle(), T("Укажите IP-адрес устройства."));
            return;
        }
        m_address = ip;
        start(m_user->text().trimmed(), m_pass->text());
    });
    connect(m_adder, &DeviceAdder::resolved, this, [this](int, const AddResult &res) { onResolved(res); });
}

void HikIpDialog::start(const QString &user, const QString &password)
{
    m_curUser = user;
    AddRequest r;
    r.address = m_address;
    r.httpPort = quint16(m_httpPort->value());
    r.rtspPort = quint16(m_rtspPort->value());
    r.user = user;
    r.password = password;
    r.channels = m_channels->value();

    m_btn->setEnabled(false);
    m_progress->setRange(0, 0);
    m_status->setText(T("Проверка устройства %1…").arg(m_address));
    m_adder->start({r}, 1);
}

void HikIpDialog::onResolved(const AddResult &res)
{
    m_progress->setRange(0, 1);
    m_progress->setValue(1);
    m_btn->setEnabled(true);

    if (res.authFailed) {
        // устройство требует пароль: спрашиваем и повторяем
        QString user = m_curUser;
        QString pass = m_pass->text();
        if (askCredentials(this, m_address, &user, &pass)) {
            m_user->setText(user);
            m_pass->setText(pass);
            start(user, pass);
        } else {
            m_status->setText(T("Добавление отменено: нужен пароль."));
        }
        return;
    }
    if (!res.ok) {
        m_status->setText(T("Не удалось добавить: %1").arg(res.error));
        return;
    }

    const QString base = !res.name.isEmpty() ? res.name : (!res.model.isEmpty() ? res.model : m_address);
    QString group = m_group->text().trimmed();
    if (group.isEmpty() && res.recorder)
        group = QStringLiteral("%1 (%2)").arg(base, m_address);

    QList<CameraEntry> entries;
    for (const AddedChannel &c : res.channels) {
        const QString chName = c.name.isEmpty() ? T("Канал %1").arg(c.number) : c.name;
        CameraEntry e;
        if (res.recorder)
            e.name = QStringLiteral("%1 — %2").arg(m_address, chName);
        else
            e.name = !m_name->text().trimmed().isEmpty() ? m_name->text().trimmed()
                     : (base == m_address ? m_address : QStringLiteral("%1 (%2)").arg(base, m_address));
        e.group = group;
        e.url = c.mainUrl;
        e.liveUrl = c.subUrl;
        e.record = true;
        entries.append(e);
    }
    m_mgr->addMany(entries);
    m_status->setText(T("Добавлено каналов: %1 (%2). Способ определения: %3")
                          .arg(entries.size()).arg(res.model.isEmpty() ? base : res.model).arg(res.method));
}

// ======================================================================
// app/icsee_import_dialog.cpp
// ======================================================================
namespace {

enum Column_icseedlg { ColCheck_icseedlg, ColAddress_icseedlg, ColState_icseedlg, ColCount_icseedlg };
constexpr int kMaxRange_icseedlg = 2048;

bool parseIpv4_icseedlg(const QString &s, quint32 *out)
{
    QHostAddress a;
    if (!a.setAddress(s.trimmed()) || a.protocol() != QAbstractSocket::IPv4Protocol)
        return false;
    *out = a.toIPv4Address();
    return true;
}

} // namespace

IcseeImportDialog::IcseeImportDialog(CameraManager *mgr, QWidget *parent) : QDialog(parent), m_mgr(mgr)
{
    setWindowTitle(T("Добавить камеры iCSee"));
    resize(820, 600);

    for (const CameraEntry &e : m_mgr->cameras())
        m_existing.insert(QUrl(e.url).host());

    m_scanner = new PortScanner(this);

    // ---- поиск ----
    auto *findBox = new QGroupBox(T("1. Адреса камер"), this);
    auto *findLay = new QHBoxLayout(findBox);
    m_from = new QLineEdit(findBox);
    m_from->setPlaceholderText(QStringLiteral("192.168.1.1"));
    m_to = new QLineEdit(findBox);
    m_to->setPlaceholderText(QStringLiteral("192.168.1.254"));
    m_port = new QSpinBox(findBox);
    m_port->setRange(1, 65535);
    m_port->setValue(554);
    m_port->setToolTip(T("RTSP-порт камеры, по умолчанию 554"));
    m_btnScan = new QPushButton(T("Найти камеры"), findBox);
    findLay->addWidget(new QLabel(T("Диапазон IP:"), findBox));
    findLay->addWidget(m_from);
    findLay->addWidget(new QLabel(QStringLiteral("—"), findBox));
    findLay->addWidget(m_to);
    findLay->addWidget(new QLabel(T("RTSP-порт"), findBox));
    findLay->addWidget(m_port);
    findLay->addWidget(m_btnScan);

    // ---- доступ ----
    auto *authBox = new QGroupBox(T("2. Логин и пароль камер"), this);
    auto *authLay = new QHBoxLayout(authBox);
    m_user = new QLineEdit(QStringLiteral("admin"), authBox);
    m_pass = new QLineEdit(authBox);
    m_pass->setEchoMode(QLineEdit::Password);
    m_pass->setPlaceholderText(T("у новых камер iCSee часто пустой"));
    m_group = new QLineEdit(authBox);
    m_group->setPlaceholderText(T("например, Двор"));
    authLay->addWidget(new QLabel(T("Логин:"), authBox));
    authLay->addWidget(m_user);
    authLay->addWidget(new QLabel(T("Пароль:"), authBox));
    authLay->addWidget(m_pass);
    authLay->addWidget(new QLabel(T("Группа:"), authBox));
    authLay->addWidget(m_group);

    // ---- таблица ----
    m_table = new QTableWidget(0, ColCount_icseedlg, this);
    m_table->setHorizontalHeaderLabels({QString(), T("Адрес"), T("Состояние")});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->verticalHeader()->setVisible(false);
    m_table->setColumnWidth(ColCheck_icseedlg, 34);
    m_table->setColumnWidth(ColAddress_icseedlg, 200);
    m_table->horizontalHeader()->setSectionResizeMode(ColState_icseedlg, QHeaderView::Stretch);

    auto *selAll = new QPushButton(T("Выбрать все"), this);
    auto *selNone = new QPushButton(T("Снять выбор"), this);
    auto *selRow = new QHBoxLayout;
    selRow->addWidget(new QLabel(T("3. Найденные адреса с открытым RTSP-портом"), this));
    selRow->addStretch(1);
    selRow->addWidget(selAll);
    selRow->addWidget(selNone);

    // ---- низ ----
    m_progress = new QProgressBar(this);
    m_progress->setRange(0, 1);
    m_progress->setValue(0);
    m_status = new QLabel(
        T("Камеры должны быть в вашей сети (по IP). Облачное подключение по серийному номеру не используется."),
        this);
    m_status->setWordWrap(true);
    m_btnAdd = new QPushButton(T("Добавить выбранные"), this);
    auto *close = new QPushButton(T("Закрыть"), this);

    auto *bottom = new QHBoxLayout;
    bottom->addWidget(m_progress, 1);
    bottom->addWidget(m_btnAdd);
    bottom->addWidget(close);

    auto *root = new QVBoxLayout(this);
    root->addWidget(findBox);
    root->addWidget(authBox);
    root->addLayout(selRow);
    root->addWidget(m_table, 1);
    root->addWidget(m_status);
    root->addLayout(bottom);

    // ---- сигналы ----
    connect(m_btnScan, &QPushButton::clicked, this, &IcseeImportDialog::startScan);
    connect(m_btnAdd, &QPushButton::clicked, this, &IcseeImportDialog::addSelected);
    connect(selAll, &QPushButton::clicked, this, [this] { setAllChecked(true); });
    connect(selNone, &QPushButton::clicked, this, [this] { setAllChecked(false); });
    connect(close, &QPushButton::clicked, this, &QDialog::accept);

    connect(m_scanner, &PortScanner::found, this, &IcseeImportDialog::addHost);
    connect(m_scanner, &PortScanner::progress, this, [this](int done, int total) {
        m_progress->setRange(0, total);
        m_progress->setValue(done);
        m_status->setText(T("Проверено %1 из %2, найдено: %3").arg(done).arg(total).arg(m_hosts.size()));
    });
    connect(m_scanner, &PortScanner::finished, this, [this] {
        m_status->setText(T("Поиск завершён. Найдено адресов: %1").arg(m_hosts.size()));
        m_btnScan->setEnabled(true);
        m_btnAdd->setEnabled(true);
    });
}

void IcseeImportDialog::startScan()
{
    quint32 a = 0, b = 0;
    if (!parseIpv4_icseedlg(m_from->text(), &a) || !parseIpv4_icseedlg(m_to->text(), &b) || b < a) {
        QMessageBox::warning(this, windowTitle(), T("Укажите корректный диапазон IPv4-адресов."));
        return;
    }
    if (quint64(b) - a + 1 > kMaxRange_icseedlg) {
        QMessageBox::warning(this, windowTitle(),
            T("Слишком большой диапазон: максимум %1 адресов за раз.").arg(kMaxRange_icseedlg));
        return;
    }
    m_btnScan->setEnabled(false);
    m_btnAdd->setEnabled(false);
    m_progress->setRange(0, int(b - a + 1));
    m_progress->setValue(0);
    m_scanner->start(a, b, quint16(m_port->value()), 64, 800);
}

void IcseeImportDialog::addHost(const QString &ip)
{
    if (m_hosts.contains(ip))
        return;
    const int row = m_table->rowCount();
    m_table->insertRow(row);
    m_hosts.append(ip);

    const bool exists = m_existing.contains(ip);
    auto *chk = new QTableWidgetItem;
    chk->setFlags(exists ? Qt::NoItemFlags : (Qt::ItemIsUserCheckable | Qt::ItemIsEnabled));
    chk->setCheckState(exists ? Qt::Unchecked : Qt::Checked);
    m_table->setItem(row, ColCheck_icseedlg, chk);
    m_table->setItem(row, ColAddress_icseedlg, new QTableWidgetItem(ip));
    m_table->setItem(row, ColState_icseedlg,
                     new QTableWidgetItem(exists ? T("Уже добавлена")
                                                 : T("RTSP-порт открыт")));
}

void IcseeImportDialog::setAllChecked(bool on)
{
    for (int row = 0; row < m_table->rowCount(); ++row) {
        QTableWidgetItem *it = m_table->item(row, ColCheck_icseedlg);
        if (it->flags() & Qt::ItemIsUserCheckable)
            it->setCheckState(on ? Qt::Checked : Qt::Unchecked);
    }
}

void IcseeImportDialog::addSelected()
{
    const QString user = m_user->text().trimmed();
    const QString pass = m_pass->text();
    const QString group = m_group->text().trimmed();
    const quint16 port = quint16(m_port->value());

    QList<CameraEntry> entries;
    QList<int> rows;
    for (int row = 0; row < m_table->rowCount(); ++row) {
        const QTableWidgetItem *chk = m_table->item(row, ColCheck_icseedlg);
        if (!(chk->flags() & Qt::ItemIsUserCheckable) || chk->checkState() != Qt::Checked)
            continue;
        const QString ip = m_hosts.at(row);

        CameraEntry e;
        e.name = QStringLiteral("iCSee %1").arg(ip);
        e.group = group;
        e.url = icseeRtspUrl(ip, port, user, pass, 1, 0);       // основной поток — в архив
        e.liveUrl = icseeRtspUrl(ip, port, user, pass, 1, 1);   // подпоток — в мозаику
        e.record = true;
        entries.append(e);
        rows.append(row);
    }

    if (entries.isEmpty()) {
        QMessageBox::information(this, windowTitle(), T("Отметьте камеры для добавления."));
        return;
    }

    m_mgr->addMany(entries);
    for (int row : rows) {
        m_table->item(row, ColState_icseedlg)->setText(T("Добавлена"));
        m_table->item(row, ColCheck_icseedlg)->setCheckState(Qt::Unchecked);
        m_table->item(row, ColCheck_icseedlg)->setFlags(Qt::NoItemFlags);
        m_existing.insert(m_hosts.at(row));
    }
    m_status->setText(T("Добавлено камер: %1. Состояние записи смотрите на вкладке «Устройства» и в «Журнале».")
                          .arg(entries.size()));
}

// ======================================================================
// app/lan_search_dialog.cpp
// ======================================================================
namespace {

enum LanColumn { LanColCheck, LanColAddress, LanColVendor, LanColModel, LanColState, LanColCount };
constexpr int kLanMaxRange = 2048;

bool lanParseIpv4(const QString &s, quint32 *out)
{
    QHostAddress a;
    if (!a.setAddress(s.trimmed()) || a.protocol() != QAbstractSocket::IPv4Protocol)
        return false;
    *out = a.toIPv4Address();
    return true;
}

} // namespace

LanSearchDialog::LanSearchDialog(CameraManager *mgr, QWidget *parent) : QDialog(parent), m_mgr(mgr)
{
    setWindowTitle(T("Поиск камер в сети: Hikvision, HiWatch, Polyvision, Dahua"));
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    resize(1020, 700);

    for (const CameraEntry &e : m_mgr->cameras())
        m_existing.insert(QUrl(e.url).host());

    m_sadp = new HikDiscovery(this);
    m_onvif = new OnvifDiscovery(this);
    m_scan = new PortScanner(this);
    m_adder = new DeviceAdder(this);

    // ---- поиск ----
    auto *findBox = new QGroupBox(T("1. Поиск устройств"), this);
    auto *findLay = new QHBoxLayout(findBox);
    m_btnFind = new QPushButton(T("Найти в своей сети"), findBox);
    m_from = new QLineEdit(findBox);
    m_from->setPlaceholderText(QStringLiteral("192.168.1.1"));
    m_to = new QLineEdit(findBox);
    m_to->setPlaceholderText(QStringLiteral("192.168.1.254"));
    m_btnScan = new QPushButton(T("Сканировать диапазон"), findBox);
    m_filter = new QComboBox(findBox);
    m_filter->addItems({T("Все производители"), QStringLiteral("Hikvision"), QStringLiteral("HiWatch"),
                        QStringLiteral("Polyvision"), QStringLiteral("Dahua")});
    findLay->addWidget(m_btnFind);
    findLay->addSpacing(12);
    findLay->addWidget(new QLabel(T("Диапазон IP:"), findBox));
    findLay->addWidget(m_from);
    findLay->addWidget(new QLabel(QStringLiteral("—"), findBox));
    findLay->addWidget(m_to);
    findLay->addWidget(m_btnScan);
    findLay->addSpacing(12);
    findLay->addWidget(m_filter);

    // ---- доступ ----
    auto *authBox = new QGroupBox(T("2. Общие логин и пароль"), this);
    auto *authLay = new QHBoxLayout(authBox);
    m_user = new QLineEdit(QStringLiteral("admin"), authBox);
    m_pass = new QLineEdit(authBox);
    m_pass->setEchoMode(QLineEdit::Password);
    m_group = new QLineEdit(authBox);
    m_group->setPlaceholderText(T("для регистратора — по умолчанию его имя"));
    m_rtsp = new QSpinBox(authBox);
    m_rtsp->setRange(1, 65535);
    m_rtsp->setValue(554);
    m_channels = new QSpinBox(authBox);
    m_channels->setRange(0, 128);
    m_channels->setSpecialValueText(T("авто"));
    m_channels->setToolTip(T("Число каналов регистратора, если автоопределение не сработало"));
    authLay->addWidget(new QLabel(T("Логин:"), authBox));
    authLay->addWidget(m_user);
    authLay->addWidget(new QLabel(T("Пароль:"), authBox));
    authLay->addWidget(m_pass);
    authLay->addWidget(new QLabel(T("Группа:"), authBox));
    authLay->addWidget(m_group, 1);
    authLay->addWidget(new QLabel(T("RTSP-порт"), authBox));
    authLay->addWidget(m_rtsp);
    authLay->addWidget(new QLabel(T("Каналов в регистраторе:"), authBox));
    authLay->addWidget(m_channels);

    // ---- таблица ----
    m_table = new QTableWidget(0, LanColCount, this);
    m_table->setHorizontalHeaderLabels({QString(), T("Адрес"), T("Производитель"), T("Модель / имя"), T("Состояние")});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->verticalHeader()->setVisible(false);
    m_table->setColumnWidth(LanColCheck, 34);
    m_table->setColumnWidth(LanColAddress, 140);
    m_table->setColumnWidth(LanColVendor, 120);
    m_table->setColumnWidth(LanColModel, 320);
    m_table->horizontalHeader()->setSectionResizeMode(LanColState, QHeaderView::Stretch);

    auto *selAll = new QPushButton(T("Выбрать все"), this);
    auto *selNone = new QPushButton(T("Снять выбор"), this);
    auto *selRow = new QHBoxLayout;
    selRow->addWidget(new QLabel(T("3. Найденные устройства"), this));
    selRow->addStretch(1);
    selRow->addWidget(selAll);
    selRow->addWidget(selNone);

    // ---- низ ----
    m_progress = new QProgressBar(this);
    m_progress->setRange(0, 1);
    m_progress->setValue(0);
    m_status = new QLabel(T("Нажмите «Найти в своей сети» или задайте диапазон IP."), this);
    m_btnImport = new QPushButton(T("Добавить выбранные"), this);
    auto *close = new QPushButton(T("Закрыть"), this);
    auto *bottom = new QHBoxLayout;
    bottom->addWidget(m_progress, 1);
    bottom->addWidget(m_btnImport);
    bottom->addWidget(close);

    auto *root = new QVBoxLayout(this);
    root->addWidget(findBox);
    root->addWidget(authBox);
    root->addLayout(selRow);
    root->addWidget(m_table, 1);
    root->addWidget(m_status);
    root->addLayout(bottom);

    connect(m_btnFind, &QPushButton::clicked, this, &LanSearchDialog::startSearch);
    connect(m_btnScan, &QPushButton::clicked, this, &LanSearchDialog::startScan);
    connect(m_btnImport, &QPushButton::clicked, this, &LanSearchDialog::importSelected);
    connect(m_filter, &QComboBox::currentIndexChanged, this, &LanSearchDialog::applyFilter);
    connect(selAll, &QPushButton::clicked, this, [this] { setAllChecked(true); });
    connect(selNone, &QPushButton::clicked, this, [this] { setAllChecked(false); });
    connect(close, &QPushButton::clicked, this, &QDialog::accept);

    // ---- поиск: SADP и ONVIF работают одновременно ----
    connect(m_sadp, &HikDiscovery::deviceFound, this, [this](const HikDevice &d) {
        addCandidate(d.address, d.httpPort, d.model, QString());
    });
    connect(m_onvif, &OnvifDiscovery::deviceFound, this, [this](const OnvifDevice &d) {
        addCandidate(d.address, 80, d.hardware, d.name);
    });
    auto searchDone = [this] {
        if (--m_pendingSearches > 0)
            return;
        m_progress->setRange(0, 1);
        m_progress->setValue(1);
        m_status->setText(T("Поиск завершён. Найдено устройств: %1. Если список пуст, используйте сканирование диапазона.")
                              .arg(m_candidates.size()));
        setBusy(false);
    };
    connect(m_sadp, &HikDiscovery::finished, this, searchDone);
    connect(m_onvif, &OnvifDiscovery::finished, this, searchDone);

    connect(m_scan, &PortScanner::found, this, [this](const QString &ip) { addCandidate(ip, 80, QString(), QString()); });
    connect(m_scan, &PortScanner::progress, this, [this](int done, int total) {
        m_progress->setRange(0, total);
        m_progress->setValue(done);
        m_status->setText(T("Проверено %1 из %2, найдено: %3").arg(done).arg(total).arg(m_candidates.size()));
    });
    connect(m_scan, &PortScanner::finished, this, [this] {
        m_status->setText(T("Сканирование завершено. Найдено адресов: %1").arg(m_candidates.size()));
        setBusy(false);
    });

    // ---- добавление ----
    connect(m_adder, &DeviceAdder::progress, this, [this](int done, int total) {
        m_progress->setRange(0, total);
        m_progress->setValue(done);
        m_status->setText(T("Опрос устройств: %1 из %2").arg(done).arg(total));
    });
    connect(m_adder, &DeviceAdder::resolved, this, [this](int i, const AddResult &res) {
        const int row = m_roundRows.at(i);
        if (res.ok) {
            addEntries(row, res);
        } else if (res.authFailed) {
            m_authRetry.append(row);
            setState(row, T("Нужен пароль"));
        } else {
            ++m_failed;
            setState(row, res.error);
        }
    });
    connect(m_adder, &DeviceAdder::finished, this, &LanSearchDialog::onRoundFinished);
}

void LanSearchDialog::addCandidate(const QString &ip, quint16 httpPort, const QString &model, const QString &name)
{
    const QString title = name.isEmpty() || name == model ? model : (model.isEmpty() ? name : model + QStringLiteral(" · ") + name);
    const QString vendor = guessVendor(model, name);

    for (int i = 0; i < m_candidates.size(); ++i) {
        if (m_candidates.at(i).address != ip)
            continue;
        if (m_candidates.at(i).model.isEmpty() && !title.isEmpty()) {   // уточняем данные, найденные другим способом
            m_candidates[i].model = title;
            m_candidates[i].vendor = vendor;
            m_table->item(i, LanColModel)->setText(title);
            m_table->item(i, LanColVendor)->setText(vendor.isEmpty() ? QStringLiteral("—") : vendor);
            applyFilter();
        }
        return;
    }

    const int row = m_table->rowCount();
    m_table->insertRow(row);
    Candidate c;
    c.address = ip;
    c.httpPort = httpPort ? httpPort : 80;
    c.model = title;
    c.vendor = vendor;
    m_candidates.append(c);

    const bool exists = m_existing.contains(ip);
    auto *chk = new QTableWidgetItem;
    chk->setFlags(exists ? Qt::NoItemFlags : (Qt::ItemIsUserCheckable | Qt::ItemIsEnabled));
    chk->setCheckState(exists ? Qt::Unchecked : Qt::Checked);
    m_table->setItem(row, LanColCheck, chk);
    m_table->setItem(row, LanColAddress, new QTableWidgetItem(ip));
    m_table->setItem(row, LanColVendor, new QTableWidgetItem(vendor.isEmpty() ? QStringLiteral("—") : vendor));
    m_table->setItem(row, LanColModel, new QTableWidgetItem(title));
    m_table->setItem(row, LanColState, new QTableWidgetItem(exists ? T("Уже добавлено") : T("Найдено")));
    applyFilter();
}

void LanSearchDialog::applyFilter()
{
    const int f = m_filter->currentIndex();   // 0 — все, 1 — Hikvision, 2 — HiWatch, 3 — Polyvision, 4 — Dahua
    const QString want = f == 1 ? QStringLiteral("Hikvision") : f == 2 ? QStringLiteral("HiWatch")
                       : f == 3 ? QStringLiteral("Polyvision") : f == 4 ? QStringLiteral("Dahua") : QString();
    for (int row = 0; row < m_table->rowCount(); ++row)
        m_table->setRowHidden(row, !want.isEmpty() && m_candidates.at(row).vendor != want);
}

void LanSearchDialog::setAllChecked(bool on)
{
    for (int row = 0; row < m_table->rowCount(); ++row) {
        QTableWidgetItem *it = m_table->item(row, LanColCheck);
        if ((it->flags() & Qt::ItemIsUserCheckable) && !m_table->isRowHidden(row))
            it->setCheckState(on ? Qt::Checked : Qt::Unchecked);
    }
}

void LanSearchDialog::setState(int row, const QString &text)
{
    if (row >= 0 && row < m_table->rowCount())
        m_table->item(row, LanColState)->setText(text);
}

void LanSearchDialog::setBusy(bool busy)
{
    m_btnFind->setEnabled(!busy);
    m_btnScan->setEnabled(!busy);
    m_btnImport->setEnabled(!busy);
}

void LanSearchDialog::startSearch()
{
    setBusy(true);
    m_pendingSearches = 2;
    m_progress->setRange(0, 0);
    m_status->setText(T("Поиск устройств в сети…"));
    m_sadp->start(4000);
    m_onvif->start(4000);
}

void LanSearchDialog::startScan()
{
    quint32 a = 0, b = 0;
    if (!lanParseIpv4(m_from->text(), &a) || !lanParseIpv4(m_to->text(), &b) || b < a) {
        QMessageBox::warning(this, windowTitle(), T("Укажите корректный диапазон IPv4-адресов."));
        return;
    }
    if (quint64(b) - a + 1 > kLanMaxRange) {
        QMessageBox::warning(this, windowTitle(), T("Слишком большой диапазон: максимум %1 адресов за раз.").arg(kLanMaxRange));
        return;
    }
    setBusy(true);
    m_progress->setRange(0, int(b - a + 1));
    m_progress->setValue(0);
    m_scan->start(a, b, quint16(m_rtsp->value()), 64, 800);   // камеры отвечают на RTSP-порту
}

void LanSearchDialog::importSelected()
{
    if (m_user->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, windowTitle(), T("Укажите логин."));
        return;
    }
    m_commonUser = m_user->text().trimmed();
    m_commonPass = m_pass->text();
    m_groupName = m_group->text().trimmed();
    m_rtspPort = quint16(m_rtsp->value());
    m_fallbackChannels = m_channels->value();

    m_roundRows.clear();
    m_rowUser.clear();
    m_rowPass.clear();
    m_authRetry.clear();
    m_entries.clear();
    m_addedRows.clear();
    m_failed = 0;
    m_devicesOk = 0;

    for (int row = 0; row < m_table->rowCount(); ++row) {
        const QTableWidgetItem *chk = m_table->item(row, LanColCheck);
        if (!(chk->flags() & Qt::ItemIsUserCheckable) || chk->checkState() != Qt::Checked || m_table->isRowHidden(row))
            continue;
        m_roundRows.append(row);
        setState(row, T("Запрос…"));
    }
    if (m_roundRows.isEmpty()) {
        QMessageBox::information(this, windowTitle(), T("Отметьте устройства для добавления."));
        return;
    }
    setBusy(true);
    runRound();
}

void LanSearchDialog::runRound()
{
    QList<AddRequest> reqs;
    for (int row : std::as_const(m_roundRows)) {
        AddRequest r;
        r.address = m_candidates.at(row).address;
        r.httpPort = m_candidates.at(row).httpPort;
        r.rtspPort = m_rtspPort;
        r.user = m_rowUser.value(row, m_commonUser);
        r.password = m_rowPass.value(row, m_commonPass);
        r.channels = m_fallbackChannels;
        reqs.append(r);
    }
    m_progress->setRange(0, int(reqs.size()));
    m_progress->setValue(0);
    m_adder->start(reqs, 6);
}

void LanSearchDialog::addEntries(int row, const AddResult &res)
{
    const QString address = m_candidates.at(row).address;
    const QString base = !res.name.isEmpty() ? res.name : (!res.model.isEmpty() ? res.model : address);
    QString group = m_groupName;
    if (group.isEmpty() && res.recorder)
        group = QStringLiteral("%1 (%2)").arg(base, address);

    for (const AddedChannel &c : res.channels) {
        const QString chName = c.name.isEmpty() ? T("Канал %1").arg(c.number) : c.name;
        CameraEntry e;
        e.name = res.recorder ? QStringLiteral("%1 — %2").arg(address, chName)
                              : (base == address ? address : QStringLiteral("%1 (%2)").arg(base, address));
        e.group = group;
        e.url = c.mainUrl;
        e.liveUrl = c.subUrl;
        e.record = true;
        m_entries.append(e);
    }
    m_addedRows.append(row);
    ++m_devicesOk;

    QString kind = res.recorder ? T("регистратор") : T("камера");
    QString model = res.model.isEmpty() ? base : res.model;
    m_table->item(row, LanColModel)->setText(T("%1 — %2, каналов: %3").arg(model, kind).arg(res.channels.size()));
    setState(row, T("Готово к добавлению") + QStringLiteral(" (") + res.method + QStringLiteral(")"));
}

void LanSearchDialog::onRoundFinished()
{
    // Устройствам, которые отказали во входе, запрашиваем логин и пароль и повторяем опрос
    if (!m_authRetry.isEmpty()) {
        QList<int> next;
        const QList<int> pending = m_authRetry;
        m_authRetry.clear();
        for (int row : pending) {
            QString user = m_rowUser.value(row, m_commonUser);
            QString pass = m_rowPass.value(row, m_commonPass);
            if (askCredentials(this, m_candidates.at(row).address, &user, &pass)) {
                m_rowUser[row] = user;
                m_rowPass[row] = pass;
                next.append(row);
                setState(row, T("Запрос…"));
            } else {
                ++m_failed;
                setState(row, T("Пропущено: нужен пароль"));
            }
        }
        if (!next.isEmpty()) {
            m_roundRows = next;
            runRound();
            return;
        }
    }
    finishImport();
}

void LanSearchDialog::finishImport()
{
    if (!m_entries.isEmpty())
        m_mgr->addMany(m_entries);
    for (int row : std::as_const(m_addedRows)) {
        setState(row, T("Добавлено"));
        m_table->item(row, LanColCheck)->setCheckState(Qt::Unchecked);
        m_table->item(row, LanColCheck)->setFlags(Qt::NoItemFlags);
        m_existing.insert(m_candidates.at(row).address);
    }
    m_status->setText(T("Добавлено каналов: %1 (устройств: %2), с ошибкой: %3%4")
                          .arg(m_entries.size()).arg(m_devicesOk).arg(m_failed)
                          .arg(m_failed ? T(" — исправьте логин/пароль и повторите") : QString()));
    m_entries.clear();
    m_addedRows.clear();
    setBusy(false);
}

// ======================================================================
// app/live_view_page.cpp
// ======================================================================
namespace {

QIcon liveDotIcon(const QColor &c)
{
    QPixmap pm(12, 12);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(c);
    p.drawEllipse(1, 1, 10, 10);
    return QIcon(pm);
}

// зелёный — идёт запись, оранжевый — подключение, синий — в сети, красный — не в сети
QColor liveStateColor(const CameraManager *mgr, const QString &id)
{
    switch (mgr->stateOf(id)) {
    case CameraWorker::State::Recording:    return QColor("#3fb950");
    case CameraWorker::State::Connecting:
    case CameraWorker::State::Reconnecting: return QColor("#e8833a");
    case CameraWorker::State::Stopped:      break;
    }
    return mgr->isOnline(id) ? QColor("#4ea1ff") : QColor("#d9534f");
}

} // namespace

LiveViewPage::LiveViewPage(CameraManager *mgr, QWidget *parent) : QWidget(parent), m_mgr(mgr)
{
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    auto *split = new QSplitter(Qt::Horizontal, this);
    root->addWidget(split);

    // ---- левая панель: поиск, дерево камер, PTZ ----
    auto *left = new QWidget;
    m_leftPanel = left;
    auto *lv = new QVBoxLayout(left);
    lv->setContentsMargins(0, 0, 0, 0);
    lv->setSpacing(0);

    m_search = new QLineEdit(left);
    m_search->setPlaceholderText(T("Поиск камеры…"));
    m_search->setClearButtonEnabled(true);

    m_tree = new CameraTree(left);
    m_tree->setHeaderHidden(true);
    m_tree->setDragEnabled(true);
    m_tree->setDragDropMode(QAbstractItemView::DragOnly);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tree->setIndentation(14);

    auto wrap = [left](QWidget *w, int l, int t, int r, int b) {
        auto *c = new QWidget(left);
        auto *cl = new QVBoxLayout(c);
        cl->setContentsMargins(l, t, r, b);
        cl->addWidget(w);
        return c;
    };
    auto *title = new QLabel(T("Камеры"), left);
    title->setObjectName(QStringLiteral("panelTitle"));
    lv->addWidget(title);
    lv->addWidget(wrap(m_search, 8, 8, 8, 6));
    lv->addWidget(m_tree, 1);
    left->setMinimumWidth(190);
    left->setMaximumWidth(260);

    // ---- центр: мозаика и нижняя панель ----
    auto *center = new QWidget;
    auto *cv = new QVBoxLayout(center);
    cv->setContentsMargins(0, 0, 0, 0);
    cv->setSpacing(0);

    m_grid = new VideoGrid(mgr, center);
    m_grid->setContentsMargins(4, 4, 4, 4);

    auto *toolbar = new QWidget(center);
    toolbar->setObjectName(QStringLiteral("videoToolbar"));
    auto *tools = new QHBoxLayout(toolbar);
    tools->setContentsMargins(10, 8, 10, 8);

    auto *panelBtn = new QToolButton(center);
    panelBtn->setText(T("☰ Камеры"));
    panelBtn->setCheckable(true);
    panelBtn->setChecked(true);
    panelBtn->setToolTip(T("Показать или скрыть список камер"));
    tools->addWidget(panelBtn);
    tools->addSpacing(8);

    // раскладка экрана: от 1 до 64 ячеек
    m_layoutBtn = new QToolButton(center);
    m_layoutBtn->setPopupMode(QToolButton::InstantPopup);
    m_layoutBtn->setToolTip(T("Раскладка экрана"));
    auto *layoutMenu = new QMenu(m_layoutBtn);
    auto *layoutGroup = new QActionGroup(layoutMenu);
    for (const GridLayoutDef &d : VideoGrid::layouts()) {
        QAction *a = layoutMenu->addAction(VideoGrid::layoutLabel(d.count));
        a->setCheckable(true);
        a->setData(d.count);
        layoutGroup->addAction(a);
        connect(a, &QAction::triggered, this, [this, d] { m_grid->setLayoutCount(d.count); });
    }
    m_layoutBtn->setMenu(layoutMenu);
    auto updateLayoutBtn = [this, layoutGroup](int count) {
        m_layoutBtn->setText(QStringLiteral("▦ ") + VideoGrid::layoutLabel(count));
        for (QAction *a : layoutGroup->actions())
            a->setChecked(a->data().toInt() == count);
    };
    connect(m_grid, &VideoGrid::layoutCountChanged, this, updateLayoutBtn);
    updateLayoutBtn(m_grid->layoutCount());
    tools->addWidget(m_layoutBtn);

    // шаблоны расположения камер
    m_tplBtn = new QToolButton(center);
    m_tplBtn->setText(T("Шаблоны"));
    m_tplBtn->setPopupMode(QToolButton::InstantPopup);
    m_tplBtn->setToolTip(T("Расположение камер сохраняется автоматически. Здесь можно сохранить именованные шаблоны."));
    auto *tplMenu = new QMenu(m_tplBtn);
    m_tplBtn->setMenu(tplMenu);
    connect(tplMenu, &QMenu::aboutToShow, this, &LiveViewPage::rebuildTemplatesMenu);
    tools->addWidget(m_tplBtn);

    auto *refreshBtn = new QToolButton(center);
    refreshBtn->setText(T("⟳ Обновить"));
    refreshBtn->setToolTip(T("Обновить окна с камерами"));
    tools->addWidget(refreshBtn);

    auto *archiveBtn = new QToolButton(center);
    archiveBtn->setText(T("▣  Архив"));
    archiveBtn->setToolTip(T("Открыть архив записей выбранной камеры"));
    archiveBtn->setCursor(Qt::PointingHandCursor);
    archiveBtn->setMinimumHeight(34);
    archiveBtn->setMinimumWidth(104);
    archiveBtn->setStyleSheet(QStringLiteral(
        "QToolButton { color:#f5f6f7; font-weight:800; padding:6px 13px; "
        "border:1px solid #a88b24; border-radius:6px; "
        "background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #555d65,stop:0.48 #343a41,stop:1 #252a30); }"
        "QToolButton:hover { color:#ffe36a; border-color:#ffe36a; "
        "background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #626a72,stop:1 #343a41); }"
        "QToolButton:pressed { color:#171a1e; background:#e9bd00; border-color:#ffe36a; }"));
    tools->addWidget(archiveBtn);
    tools->addSpacing(12);

    auto *startBtn = new QToolButton(center);
    startBtn->setObjectName(QStringLiteral("toolbarStartBtn"));
    startBtn->setText(T("Запустить"));
    startBtn->setIcon(makeCellIcon(QStringLiteral("play"), QColor("#f3d36c")));
    startBtn->setIconSize(QSize(17, 17));
    startBtn->setToolTip(T("Запустить показ камер в ячейках и запись"));
    auto *stopBtn = new QToolButton(center);
    stopBtn->setObjectName(QStringLiteral("toolbarStopBtn"));
    stopBtn->setText(T("Остановить показ"));
    stopBtn->setIcon(makeCellIcon(QStringLiteral("stop"), QColor("#f3d36c")));
    stopBtn->setIconSize(QSize(17, 17));
    stopBtn->setToolTip(T("Остановить показ в ячейках (запись продолжается)"));
    tools->addWidget(startBtn);
    tools->addWidget(stopBtn);

    tools->addStretch(1);
    auto *closeAll = new QPushButton(T("Закрыть все"), center);
    tools->addWidget(closeAll);

    cv->addWidget(m_grid, 1);
    cv->addWidget(toolbar);

    // Правая панель управления PTZ — как на фирменном макете PSVMS
    auto *rightPanel = new QWidget;
    rightPanel->setObjectName(QStringLiteral("controlRail"));
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    auto *ptzTitle = new QLabel(T("Управление PTZ"), rightPanel);
    ptzTitle->setObjectName(QStringLiteral("panelTitle"));
    rightLayout->addWidget(ptzTitle);
    auto *ptz = new PtzPanel(mgr, rightPanel);
    ptz->setEnabled(isProVersion());
    ptz->setToolTip(isProVersion() ? QString() : T("Управление PTZ доступно в Pro-версии"));
    rightLayout->addWidget(ptz);
    rightLayout->addStretch(1);
    rightPanel->setMinimumWidth(205);
    rightPanel->setMaximumWidth(260);

    split->addWidget(left);
    split->addWidget(center);
    split->addWidget(rightPanel);
    split->setStretchFactor(0, 0);
    split->setStretchFactor(1, 1);
    split->setStretchFactor(2, 0);
    split->setCollapsible(0, false);
    split->setCollapsible(1, false);
    split->setCollapsible(2, false);
    split->setSizes({220, 900, 230});

    // ---- сигналы ----
    connect(closeAll, &QPushButton::clicked, m_grid, &VideoGrid::clearAll);
    connect(startBtn, &QToolButton::clicked, this, [this] {
        m_grid->startAllCells();
        m_mgr->startAll();
    });
    connect(stopBtn, &QToolButton::clicked, m_grid, &VideoGrid::stopAllCells);
    connect(archiveBtn, &QToolButton::clicked, this, [this] { openArchive(m_grid->selectedCameraId()); });
    connect(refreshBtn, &QToolButton::clicked, this, [this] {
        m_grid->refreshCells();
        m_mgr->probeNow();
    });
    connect(panelBtn, &QToolButton::toggled, this, &LiveViewPage::setPanelVisible);
    connect(m_search, &QLineEdit::textChanged, this, &LiveViewPage::applyFilter);
    connect(m_tree, &QWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        QTreeWidgetItem *item = m_tree->itemAt(pos);
        if (!item) return;
        const QString cameraId = item->data(0, Qt::UserRole).toString();
        const bool isGroup = cameraId.isEmpty();
        const QString groupName = item->text(0);
        QList<CameraEntry> targets;
        if (isGroup) {
            for (const CameraEntry &e : m_mgr->cameras()) {
                const QString shownGroup = e.group.isEmpty() ? T("Без группы") : e.group;
                if (shownGroup == groupName) targets.append(e);
            }
        } else if (const CameraEntry *e = m_mgr->find(cameraId)) {
            targets.append(*e);
        }
        if (targets.isEmpty()) return;

        QMenu menu(this);
        if (isGroup) {
            QAction *renameGroup = menu.addAction(T("Переименовать группу…"));
            connect(renameGroup, &QAction::triggered, this, [this, targets, groupName] {
                bool ok = false;
                const QString newName = QInputDialog::getText(
                    this, T("Переименовать группу"),
                    T("Новое название группы «%1»:").arg(groupName),
                    QLineEdit::Normal, groupName, &ok).trimmed();
                if (!ok || newName.isEmpty() || newName == groupName)
                    return;
                for (const CameraEntry &entry : targets) {
                    if (const CameraEntry *current = m_mgr->find(entry.id)) {
                        CameraEntry updated = *current;
                        updated.group = newName;
                        m_mgr->update(updated);
                    }
                }
            });
        } else {
            QAction *renameCamera = menu.addAction(T("Переименовать камеру…"));
            connect(renameCamera, &QAction::triggered, this, [this, cameraId] {
                const CameraEntry *current = m_mgr->find(cameraId);
                if (!current)
                    return;
                bool ok = false;
                const QString newName = QInputDialog::getText(
                    this, T("Переименовать камеру"), T("Новое название камеры:"),
                    QLineEdit::Normal, current->name, &ok).trimmed();
                if (!ok || newName.isEmpty() || newName == current->name)
                    return;
                CameraEntry updated = *current;
                updated.name = newName;
                m_mgr->update(updated);
            });

            menu.addSeparator();
            QAction *deleteCamera = menu.addAction(T("Удалить эту камеру / канал регистратора…"));
            deleteCamera->setEnabled(Users::current().isAdmin());
            connect(deleteCamera, &QAction::triggered, this, [this, cameraId] {
                const CameraEntry *current = m_mgr->find(cameraId);
                if (!current)
                    return;
                const QString deviceName = current->name;
                QMessageBox box(QMessageBox::Warning, T("Удаление устройства"),
                    T("Удалить «%1» из PSVMS?\nЗапись будет остановлена, устройство исчезнет из списка. Уже сохранённые файлы архива останутся на диске.").arg(deviceName),
                    QMessageBox::NoButton, this);
                QPushButton *removeButton = box.addButton(T("Удалить"), QMessageBox::DestructiveRole);
                box.addButton(T("Отмена"), QMessageBox::RejectRole);
                box.exec();
                if (box.clickedButton() != removeButton)
                    return;
                m_mgr->remove(cameraId);
            });
        }
        menu.addSeparator();
        bool allFilled = true;
        for (const CameraEntry &e : targets) allFilled = allFilled && e.fillCell;
        const QString actionText = isGroup
            ? T("Заполнить ячейки всей группы (обрезать края)")
            : T("Заполнить всю ячейку (обрезать края)");
        QAction *toggle = menu.addAction(actionText);
        toggle->setCheckable(true);
        toggle->setChecked(allFilled);
        connect(toggle, &QAction::triggered, this, [this, targets, allFilled] {
            for (const CameraEntry &entry : targets) {
                if (const CameraEntry *current = m_mgr->find(entry.id)) {
                    CameraEntry updated = *current;
                    updated.fillCell = !allFilled;
                    m_mgr->update(updated);
                }
            }
        });
        if (isGroup) {
            QAction *disable = menu.addAction(T("Отключить заполнение для группы"));
            connect(disable, &QAction::triggered, this, [this, targets] {
                for (const CameraEntry &entry : targets) {
                    if (const CameraEntry *current = m_mgr->find(entry.id)) {
                        CameraEntry updated = *current; updated.fillCell = false; m_mgr->update(updated);
                    }
                }
            });
        }
        menu.exec(m_tree->viewport()->mapToGlobal(pos));
    });
    connect(m_tree, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem *it) {
        const QString id = it->data(0, Qt::UserRole).toString();
        if (!id.isEmpty())
            m_grid->openCamera(id);
    });
    connect(m_grid, &VideoGrid::archiveRequested, this, &LiveViewPage::openArchive);
    connect(m_grid, &VideoGrid::addCameraRequested, this, &LiveViewPage::addCameraRequested);
    connect(m_grid, &VideoGrid::message, this, &LiveViewPage::message);
    connect(m_grid, &VideoGrid::selectionChanged, ptz, &PtzPanel::setCamera);
    connect(m_mgr, &CameraManager::camerasChanged, this, &LiveViewPage::rebuildTree);
    connect(m_mgr, &CameraManager::cameraStateChanged, this, &LiveViewPage::refreshStates);
    connect(m_mgr, &CameraManager::onlineChanged, this, &LiveViewPage::refreshStates);

    rebuildTree();
    m_grid->restoreLayout();     // камеры возвращаются на свои места (без запуска, если отключён автозапуск)
}

void LiveViewPage::openArchive(const QString &cameraId)
{
    auto *w = new PlaybackWindow(m_mgr, cameraId, this);
    w->show();
}

void LiveViewPage::openArchiveForSelectedCamera()
{
    openArchive(m_grid ? m_grid->selectedCameraId() : QString());
}


void LiveViewPage::rebuildTemplatesMenu()
{
    QMenu *menu = m_tplBtn->menu();
    menu->clear();
    connect(menu->addAction(T("Сохранить текущее расположение как шаблон…")), &QAction::triggered, this, [this] {
        QInputDialog dlg(this);
        dlg.setWindowTitle(T("Новый шаблон"));
        dlg.setLabelText(T("Название шаблона:"));
        dlg.setOkButtonText(T("ОК"));
        dlg.setCancelButtonText(T("Отмена"));
        if (dlg.exec() == QDialog::Accepted && !dlg.textValue().trimmed().isEmpty())
            m_grid->saveTemplate(dlg.textValue().trimmed());
    });

    const QStringList names = m_grid->templateNames();
    if (!names.isEmpty()) {
        menu->addSeparator();
        for (const QString &n : names)
            connect(menu->addAction(n), &QAction::triggered, this, [this, n] { m_grid->applyTemplateNamed(n); });
        menu->addSeparator();
        QMenu *del = menu->addMenu(T("Удалить шаблон"));
        for (const QString &n : names)
            connect(del->addAction(n), &QAction::triggered, this, [this, n] { m_grid->removeTemplate(n); });
    }
}

void LiveViewPage::rebuildTree()
{
    m_tree->clear();
    m_items.clear();

    QMap<QString, QTreeWidgetItem *> groups;
    for (const CameraEntry &e : m_mgr->cameras()) {
        const QString g = e.group.isEmpty() ? T("Без группы") : e.group;
        QTreeWidgetItem *parent = groups.value(g, nullptr);
        if (!parent) {
            parent = new QTreeWidgetItem(m_tree, QStringList(g));
            parent->setFlags(Qt::ItemIsEnabled);
            groups.insert(g, parent);
        }
        auto *it = new QTreeWidgetItem(parent, QStringList(e.name));
        it->setData(0, Qt::UserRole, e.id);
        it->setIcon(0, liveDotIcon(liveStateColor(m_mgr, e.id)));
        it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
        m_items.insert(e.id, it);
    }
    m_tree->expandAll();
    applyFilter(m_search->text());
}

void LiveViewPage::refreshStates()
{
    for (auto it = m_items.cbegin(); it != m_items.cend(); ++it)
        it.value()->setIcon(0, liveDotIcon(liveStateColor(m_mgr, it.key())));
}

void LiveViewPage::applyFilter(const QString &text)
{
    const QString t = text.trimmed();
    for (int g = 0; g < m_tree->topLevelItemCount(); ++g) {
        QTreeWidgetItem *grp = m_tree->topLevelItem(g);
        int visible = 0;
        for (int i = 0; i < grp->childCount(); ++i) {
            QTreeWidgetItem *c = grp->child(i);
            const bool show = t.isEmpty() || c->text(0).contains(t, Qt::CaseInsensitive);
            c->setHidden(!show);
            if (show)
                ++visible;
        }
        grp->setHidden(visible == 0);
    }
}

void LiveViewPage::setPanelVisible(bool on)
{
    if (m_panelAnim)
        m_panelAnim->stop();

    m_leftPanel->setMinimumWidth(0);
    if (on)
        m_leftPanel->show();

    const bool animate = Cfg::view().animations;
    auto *anim = new QPropertyAnimation(m_leftPanel, "maximumWidth", this);
    anim->setDuration(animate ? 240 : 0);
    anim->setEasingCurve(QEasingCurve::InOutCubic);
    anim->setStartValue(m_leftPanel->width());
    anim->setEndValue(on ? 300 : 0);
    connect(anim, &QPropertyAnimation::finished, this, [this, on] {
        if (on) {
            m_leftPanel->setMinimumWidth(230);
            m_leftPanel->setMaximumWidth(380);
        } else {
            m_leftPanel->hide();
        }
    });
    m_panelAnim = anim;
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// ======================================================================
// app/security_audit.cpp
// ======================================================================
namespace SecurityAudit {
void write(const QString &event, const QString &details)
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(dir);
    QFile f(dir + QStringLiteral("/security-audit.log"));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) return;
    const QString safe = details; // caller must never pass credentials or tokens
    const QByteArray line = QStringLiteral("%1 | user=%2 | %3 | %4\n")
        .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs),
             Users::current().name.isEmpty() ? QStringLiteral("anonymous") : Users::current().name,
             event, safe).toUtf8();
    f.write(line);
    f.close();
}
}

// app/login_dialog.cpp
// ======================================================================
namespace {

// Защита от подбора: после 5 неверных попыток вход блокируется на 30 секунд и дольше
int g_loginFailures = 0;
qint64 g_loginLockedUntilMs = 0;

QLabel *loginMessageLabel(QWidget *parent)
{
    auto *l = new QLabel(parent);
    l->setWordWrap(true);
    l->setStyleSheet(QStringLiteral("color:#e8833a;"));
    return l;
}

class LoginDialog : public QDialog {
public:
    explicit LoginDialog(QWidget *parent) : QDialog(parent)
    {
        setWindowTitle(App::title() + QStringLiteral(" — ") + T("вход"));
        setWindowFlag(Qt::WindowContextHelpButtonHint, false);
        setMinimumWidth(440);

        auto *hint = new QLabel(T("Программа запускается в гостевом режиме. Выберите Гостя или другую учётную запись."), this);
        hint->setWordWrap(true);

        m_user = new QComboBox(this);
        for (const UserInfo &u : Users::all()) {
            if (!u.enabled)
                continue; // отключённые учётные записи нельзя выбрать для входа
            const QString role = u.isDeveloper() ? T("разработчик")
                               : u.isGuest() ? T("гость")
                               : u.isAdmin() ? T("администратор") : T("оператор");
            m_user->addItem(u.name + QStringLiteral("   (") + role + QStringLiteral(")"), u.id);
        }
        if (m_user->count() == 0)
            m_user->setEnabled(false);
        for (int i = 0; i < m_user->count(); ++i) {
            if (Users::find(m_user->itemData(i).toString()).isGuest()) {
                m_user->setCurrentIndex(i);
                break;
            }
        }

        m_passRow = passwordFieldWithEye(this, &m_pass);
        m_remember = new QCheckBox(T("Запомнить меня на этом компьютере"), this);
        m_msg = loginMessageLabel(this);
        if (m_user->count() == 0)
            m_msg->setText(T("Нет активных учётных записей. Обратитесь к администратору."));

        m_form = new QFormLayout;
        m_form->addRow(T("Пользователь"), m_user);
        m_form->addRow(T("Пароль"), m_passRow);
        m_form->addRow(QString(), m_remember);

        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        buttons->button(QDialogButtonBox::Ok)->setText(T("Войти"));
        buttons->button(QDialogButtonBox::Cancel)->setText(T("Выход"));
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

        auto *lay = new QVBoxLayout(this);
        lay->addWidget(hint);
        lay->addLayout(m_form);
        lay->addWidget(m_msg);
        lay->addWidget(buttons);

        connect(m_user, &QComboBox::currentIndexChanged, this, [this] { updatePasswordRow(); });
        updatePasswordRow();
    }

    void accept() override
    {
        const QString id = m_user->currentData().toString();
        const UserInfo u = Users::find(id);
        if (!u.valid() || !u.enabled) {
            m_msg->setText(T("Эта учётная запись отключена."));
            return;
        }

        if (u.hasPassword) {
            const qint64 now = QDateTime::currentMSecsSinceEpoch();
            QSettings security;
            const QString key = QStringLiteral("security/login/%1/").arg(id);
            const int failures = security.value(key + QStringLiteral("failures"), 0).toInt();
            const qint64 lockedUntil = security.value(key + QStringLiteral("lockedUntil"), 0).toLongLong();
            if (now < lockedUntil) {
                m_msg->setText(T("Слишком много попыток. Подождите %1 с.").arg((lockedUntil - now + 999) / 1000));
                SecurityAudit::write(QStringLiteral("login_blocked"), id);
                return;
            }
            if (!Users::verify(id, m_pass->text())) {
                const int nextFailures = failures + 1;
                const qint64 delay = nextFailures >= 5 ? qMin<qint64>(900000, 30000LL << qMin(4, nextFailures - 5)) : 0;
                security.setValue(key + QStringLiteral("failures"), nextFailures);
                if (delay) security.setValue(key + QStringLiteral("lockedUntil"), now + delay);
                security.sync();
                SecurityAudit::write(QStringLiteral("login_failed"), id);
                m_msg->setText(delay ? T("Неверный пароль. Вход временно заблокирован.") : T("Неверный пароль."));
                m_pass->clear();
                m_pass->setFocus();
                return;
            }
            security.remove(key);
            SecurityAudit::write(QStringLiteral("login_success"), id);
        }
        g_loginFailures = 0;
        Users::setCurrent(id);
        Users::setLastUserId(id);
        if (u.hasPassword && m_remember->isChecked())
            Users::issueRememberToken(id);       // ключ входа на этом компьютере (пароль не сохраняется)
        else
            Users::forgetRemembered();
        QDialog::accept();
    }

private:
    void updatePasswordRow()
    {
        const UserInfo u = Users::find(m_user->currentData().toString());
        m_form->setRowVisible(m_passRow, u.hasPassword);
        m_form->setRowVisible(m_remember, u.hasPassword);
        m_msg->clear();
        if (u.hasPassword)
            m_pass->setFocus();
    }

    QComboBox *m_user = nullptr;
    QLineEdit *m_pass = nullptr;
    QWidget *m_passRow = nullptr;
    QCheckBox *m_remember = nullptr;
    QLabel *m_msg = nullptr;
    QFormLayout *m_form = nullptr;
};

class ChangePasswordDialog : public QDialog {
public:
    explicit ChangePasswordDialog(QWidget *parent) : QDialog(parent)
    {
        setWindowTitle(T("Смена пароля"));
        setWindowFlag(Qt::WindowContextHelpButtonHint, false);
        setMinimumWidth(420);

        m_has = Users::current().hasPassword;
        QWidget *oldRow = passwordFieldWithEye(this, &m_old);
        QWidget *newRow = passwordFieldWithEye(this, &m_new);
        QWidget *new2Row = passwordFieldWithEye(this, &m_new2);
        m_msg = loginMessageLabel(this);

        auto *hint = new QLabel(T("Оставьте новый пароль пустым, чтобы входить без пароля."), this);
        hint->setWordWrap(true);
        hint->setStyleSheet(QStringLiteral("color:#8a8a8e;"));

        auto *form = new QFormLayout;
        if (m_has)
            form->addRow(T("Текущий пароль"), oldRow);
        form->addRow(T("Новый пароль"), newRow);
        form->addRow(T("Повторите новый"), new2Row);

        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        localizeButtons(buttons);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

        auto *lay = new QVBoxLayout(this);
        lay->addWidget(hint);
        lay->addLayout(form);
        lay->addWidget(m_msg);
        lay->addWidget(buttons);
    }

    void accept() override
    {
        const QString id = Users::current().id;
        if (m_has && !Users::verify(id, m_old->text())) {
            m_msg->setText(T("Текущий пароль указан неверно."));
            return;
        }
        if (m_new->text() != m_new2->text()) {
            m_msg->setText(T("Новые пароли не совпадают."));
            return;
        }
        QString err;
        if (!Users::setPassword(id, m_new->text(), &err)) {
            m_msg->setText(err);
            return;
        }
        Users::forgetRemembered();
        QDialog::accept();
    }

private:
    bool m_has = false;
    QLineEdit *m_old = nullptr;
    QLineEdit *m_new = nullptr;
    QLineEdit *m_new2 = nullptr;
    QLabel *m_msg = nullptr;
};

} // namespace

bool authenticate(QWidget *parent)
{
    Users::ensureDefault();
    if (Users::current().valid())
        return true;                                   // уже вошли (например, после смены языка)

    Users::forgetRemembered(); // выбор учётной записи показывается при каждом запуске
    LoginDialog dlg(parent);
    if (dlg.exec() != QDialog::Accepted)
        return false;
    const UserInfo selected = Users::current();
    if (selected.isDeveloper() && !selected.hasPassword) {
        QMessageBox::information(parent, T("Первый вход разработчика"),
            T("Для учётной записи «Разработчик» необходимо создать пароль. Он будет запрашиваться при каждом входе."));
        ChangePasswordDialog setup(parent);
        if (setup.exec() != QDialog::Accepted) {
            Users::setCurrent(QString());
            return false;
        }
    } else if (selected.role == QLatin1String("admin") && !selected.hasPassword) {
        const auto answer = QMessageBox::question(parent, T("Защита Administrator"),
            T("Рекомендуется установить пароль для Administrator. Установить пароль сейчас?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (answer == QMessageBox::Yes) {
            ChangePasswordDialog setup(parent);
            if (setup.exec() != QDialog::Accepted) {
                Users::setCurrent(QString());
                return false;
            }
        }
    }
    SecurityAudit::write(QStringLiteral("session_started"), selected.id);
    return true;
}

void changeOwnPassword(QWidget *parent)
{
    ChangePasswordDialog dlg(parent);
    if (dlg.exec() == QDialog::Accepted)
        QMessageBox::information(parent, T("Смена пароля"), T("Пароль изменён."));
}

// ======================================================================
// app/main_window.cpp
// ======================================================================
namespace {

enum MainPage { PageLive = 0, PageDevices = 1, PageEvents = 2, PageSettings = 3 };

struct MainWinSize {
    int w;
    int h;
};
const MainWinSize kMainWinSizes[] = {{0, 0},       {1280, 720},  {1600, 900},  {1920, 1080},
                                     {2560, 1080}, {2560, 1440}, {3440, 1440}, {3840, 2160}};

class ButtonMotionFilter final : public QObject
{
public:
    using QObject::QObject;

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        auto *w = qobject_cast<QWidget *>(obj);
        if (!w || !(qobject_cast<QPushButton *>(w) || qobject_cast<QToolButton *>(w)))
            return QObject::eventFilter(obj, event);

        qreal target = 1.0;
        int duration = 130;
        switch (event->type()) {
        case QEvent::Enter: target = 0.96; duration = 150; break;
        case QEvent::Leave: target = 1.0; duration = 170; break;
        case QEvent::MouseButtonPress: target = 0.86; duration = 75; break;
        case QEvent::MouseButtonRelease: target = 0.98; duration = 145; break;
        default: return QObject::eventFilter(obj, event);
        }

        auto *effect = qobject_cast<QGraphicsOpacityEffect *>(w->graphicsEffect());
        if (!effect) {
            effect = new QGraphicsOpacityEffect(w);
            effect->setOpacity(1.0);
            w->setGraphicsEffect(effect);
        }
        auto *animation = new QPropertyAnimation(effect, "opacity", this);
        animation->setDuration(Cfg::view().animations ? duration : 0);
        animation->setStartValue(effect->opacity());
        animation->setEndValue(target);
        animation->setEasingCurve(QEasingCurve::OutCubic);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
        return QObject::eventFilter(obj, event);
    }
};

} // namespace

static QPixmap makePsvmsBrandWallpaper();

class PsvmsBackgroundWidget final : public QWidget
{
public:
    explicit PsvmsBackgroundWidget(QWidget *parent = nullptr)
        : QWidget(parent), m_wallpaper(makePsvmsBrandWallpaper())
    {
        setObjectName(QStringLiteral("centralWidget"));
        setAttribute(Qt::WA_StyledBackground, true);
    }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::SmoothPixmapTransform, true);
        if (m_wallpaper.isNull()) { p.fillRect(rect(), QColor("#090a0c")); return; }
        const QSize scaled = m_wallpaper.size().scaled(size(), Qt::KeepAspectRatioByExpanding);
        const QPixmap fitted = m_wallpaper.scaled(scaled, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        const QRect target((width()-fitted.width())/2, (height()-fitted.height())/2, fitted.width(), fitted.height());
        p.drawPixmap(target, fitted);
        p.fillRect(rect(), QColor(5, 6, 8, 58));
    }
private:
    QPixmap m_wallpaper;
};

MainWindow::MainWindow(CameraManager *mgr, QWidget *parent) : QMainWindow(parent), m_mgr(mgr)
{
    setWindowTitle(App::title());
    setWindowIcon(makeAppIcon());
    if (qApp) qApp->setWindowIcon(makeAppIcon());
    setMinimumSize(1280, 720);

    m_nav = new SideNav(this);
    m_top = new TopBar(mgr, this);
    m_stack = new QStackedWidget(this);

    m_live = new LiveViewPage(mgr, this);
    m_devices = new DevicesPage(mgr, this);
    m_events = new EventsPage(this);
    m_settings = new SettingsPage(mgr, this);
    m_stack->addWidget(m_live);
    m_stack->addWidget(m_devices);
    m_stack->addWidget(m_events);
    m_stack->addWidget(m_settings);

    m_nav->addPage(QStringLiteral("▣"), T("Главная"));
    m_nav->addPage(QStringLiteral("▤"), T("Камеры"));
    const int archiveNavItem = m_nav->addAction(QStringLiteral("▧"), T("Архив"));
    m_nav->addPage(QStringLiteral("⚑"), T("События"));
    const int usersNavItem = m_nav->addAction(QStringLiteral("♟"), T("Пользователи"));
    const int settingsNavItem = m_nav->addAction(QStringLiteral("⚙"), T("Настройки"));
    const int aboutNavItem = m_nav->addAction(QStringLiteral("ⓘ"), T("О программе"));
    const int exitNavItem = m_nav->addBottomAction(QStringLiteral("↪"), T("Выход"));
    if (!isProVersion())
        m_nav->setItemVisible(PageEvents, false);
    m_settingsItem = settingsNavItem;
    m_langItem = -1;
    m_fullscreenItem = -1;

    const UserInfo user = Users::current();
    m_nav->setUser(user.name, user.isDeveloper() ? T("Разработчик")
                         : user.isGuest() ? T("Гость")
                         : user.isAdmin() ? T("Администратор") : T("Оператор"));
    if (!user.isAdmin()) {                         // Гость и оператор: без настроек и управления камерами
        m_nav->setItemVisible(PageDevices, false);
        m_nav->setItemVisible(usersNavItem, false);
        m_nav->setItemVisible(settingsNavItem, false);
        m_top->setSettingsVisible(false);
    }

    connect(m_nav, &SideNav::currentChanged, this, [this](int i) {
        m_stack->setCurrentIndex(i);
        m_top->setTitle(pageTitle(i));
    });
    connect(m_nav, &SideNav::actionTriggered, this, [this, archiveNavItem, usersNavItem,
                                                        settingsNavItem, aboutNavItem, exitNavItem](int i) {
        if (i == archiveNavItem) {
            m_live->openArchiveForSelectedCamera();
            return;
        }
        if (i == usersNavItem || i == settingsNavItem || i == aboutNavItem) {
            if (!Users::current().isAdmin()) {
                statusBar()->showMessage(T("Этот раздел доступен только администратору."), 5000);
                return;
            }
            m_stack->setCurrentIndex(PageSettings);
            m_top->setTitle(pageTitle(PageSettings));
            if (auto *tabs = m_settings->findChild<QTabWidget *>()) {
                const QString wanted = i == usersNavItem ? T("Пользователи")
                                      : i == aboutNavItem ? T("О программе") : T("Общие");
                for (int tab = 0; tab < tabs->count(); ++tab) {
                    if (tabs->tabText(tab).compare(wanted, Qt::CaseInsensitive) == 0) {
                        tabs->setCurrentIndex(tab);
                        break;
                    }
                }
            }
            return;
        }
        if (i == exitNavItem) {
            App::requestLogout();
            return;
        }
    });
    connect(m_nav, &SideNav::changePasswordRequested, this, [this] { changeOwnPassword(this); });
    connect(m_nav, &SideNav::logoutRequested, this, [] {
        SecurityAudit::write(QStringLiteral("session_switch_requested"));
        App::requestLogout();
    });
    m_nav->setCurrent(PageLive, false);

    // Периодически перепроверяем срок лицензии, чтобы Pro-функции отключались
    // и при истечении срока во время открытого сеанса.
    auto *licenseTimer = new QTimer(this);
    licenseTimer->setInterval(30000);
    const bool initialProState = isProVersion();
    connect(licenseTimer, &QTimer::timeout, this, [this, previousProState = initialProState]() mutable {
        const bool currentProState = isProVersion();
        if (previousProState && !currentProState) {
            m_mgr->enforceDemoLimit();
            m_nav->setItemVisible(PageEvents, false);
            if (m_settings) {
                if (auto *tabs = m_settings->findChild<QTabWidget *>())
                    for (int i = 1; i <= 4; ++i) tabs->setTabEnabled(i, false);
            }
            statusBar()->showMessage(T("Срок лицензии истёк. Включён демонстрационный режим."), 10000);
        }
        previousProState = currentProState;
    });
    licenseTimer->start();

    connect(m_top, &TopBar::settingsRequested, this, [this] {
        if (!Users::current().isAdmin()) {
            statusBar()->showMessage(T("Настройки доступны только администратору."), 5000);
            return;
        }
        m_stack->setCurrentIndex(PageSettings);
        m_top->setTitle(pageTitle(PageSettings));
    });
    connect(m_top, &TopBar::fullscreenRequested, this, [this] { toggleMainFullscreen(); });
    connect(m_top, &TopBar::statusRequested, this, [this] {
        // Не создаём дубликаты: если окно уже открыто, активируем существующее.
        if (m_cameraStatusDialog) {
            m_cameraStatusDialog->show();
            m_cameraStatusDialog->raise();
            m_cameraStatusDialog->activateWindow();
            return;
        }
        m_cameraStatusDialog = new CameraStatusDialog(m_mgr, this);
        m_cameraStatusDialog->show();
        m_cameraStatusDialog->raise();
        m_cameraStatusDialog->activateWindow();
    });
    connect(m_top, &TopBar::cloudRequested, this, [this] {
        CloudDialog dlg(this);
        dlg.exec();
    });
    connect(m_top, &TopBar::simpleModeToggled, this, [](bool simple) {
        QSettings().setValue(QStringLiteral("perf/lowPower"), simple);
        Cfg::reload();
    });

    // «+» в пустой ячейке → «Добавить новую камеру…»
    connect(m_live, &LiveViewPage::addCameraRequested, this, [this] {
        if (Users::current().isAdmin()) {
            m_nav->setCurrent(PageDevices);
            m_devices->addCameraManually();
        } else {
            statusBar()->showMessage(T("Добавлять камеры может только администратор"), 5000);
        }
    });
    connect(m_live, &LiveViewPage::message, this, [this](const QString &text) { statusBar()->showMessage(text, 7000); });
    connect(m_settings, &SettingsPage::languageChosen, this, &MainWindow::applyLanguage);
    connect(m_settings, &SettingsPage::proSessionActivated, this, [this] {
        m_nav->setItemVisible(PageEvents, true);
        if (auto *ptz = m_live->findChild<PtzPanel *>()) {
            ptz->setEnabled(true);
            ptz->setToolTip(QString());
        }
    });
    connect(m_settings, &SettingsPage::proSessionDeactivated, this, [this] {
        m_nav->setItemVisible(PageEvents, false);
        if (m_stack->currentIndex() == PageEvents)
            m_nav->setCurrent(PageLive, false);
        if (auto *ptz = m_live->findChild<PtzPanel *>()) {
            ptz->setEnabled(false);
            ptz->setToolTip(T("Управление PTZ доступно в режиме разработчика"));
        }
    });

    auto *central = new PsvmsBackgroundWidget(this);
    auto *root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    auto *right = new QVBoxLayout;
    right->setContentsMargins(0, 0, 0, 0);
    right->setSpacing(0);
    right->addWidget(m_top);
    right->addWidget(m_stack, 1);
    root->addWidget(m_nav);
    root->addLayout(right, 1);
    setCentralWidget(central);

    // ---- строка состояния: справа — загрузка CPU/ОЗУ и часы ----
    m_sysLabel = new QLabel(this);
    m_sysLabel->setObjectName(QStringLiteral("sysStats"));
    m_clock = new QLabel(this);
    m_clock->setObjectName(QStringLiteral("clock"));
    statusBar()->addPermanentWidget(new QWidget(this), 1);   // растягивающийся пустой блок слева
    auto *editionFooter = new QLabel(this);
    editionFooter->setObjectName(QStringLiteral("editionFooter"));
    editionFooter->setStyleSheet(QStringLiteral("color:#ff4d4f;font-size:13px;font-weight:900;padding:0 12px;letter-spacing:0.4px;"));
    auto refreshEditionFooter = [editionFooter] {
        const bool pro = isProVersion();
        editionFooter->setText(pro ? T("Pro версия") : T("DEMO ВЕРСИЯ"));
        editionFooter->setStyleSheet(pro
            ? QStringLiteral("color:#e5c45a;font-size:12px;font-weight:800;padding:0 12px;")
            : QStringLiteral("color:#ff4548;font-size:13px;font-weight:900;padding:0 12px;letter-spacing:0.4px;"));
    };
    refreshEditionFooter();
    auto *demoPulse = new QTimer(editionFooter);
    demoPulse->setInterval(140);
    auto *demoPhase = new int(0);
    QObject::connect(demoPulse, &QTimer::timeout, editionFooter, [editionFooter, demoPhase] {
        *demoPhase = (*demoPhase + 5) % 360;
        const QColor glow = QColor::fromHsv((355 + (*demoPhase % 6)) % 360, 210, 255);
        editionFooter->setStyleSheet(QStringLiteral(
            "color:%1;font-size:13px;font-weight:900;padding:0 12px;letter-spacing:1px;")
            .arg(glow.name()));
    });
    auto updateDemoAnimation = [demoPulse, refreshEditionFooter] {
        refreshEditionFooter();
        if (!isProVersion() && Cfg::view().animations) demoPulse->start();
        else demoPulse->stop();
    };
    updateDemoAnimation();
    QObject::connect(Cfg::notifier(), &CfgNotifier::changed, editionFooter, updateDemoAnimation);
    statusBar()->addPermanentWidget(editionFooter);
    // Голосовое управление работает через встроенный в Windows движок System.Speech.
    m_voiceButton = new QPushButton(T("🎙 Голос"), this);
    m_voiceButton->setObjectName(QStringLiteral("voiceControlButton"));
    m_voiceButton->setToolTip(T("Включить или выключить голосовое управление на русском языке"));
    m_voiceButton->setCursor(Qt::PointingHandCursor);
    m_voiceButton->setStyleSheet(QStringLiteral(
        "QPushButton{background:#253449;color:#eaf2ff;border:1px solid #45617f;border-radius:8px;"
        "padding:5px 12px;font-weight:700;}"
        "QPushButton:hover{background:#304965;}"
        "QPushButton:checked{background:#8b3038;border-color:#e45b66;}"));
    m_voiceButton->setCheckable(true);
    statusBar()->addPermanentWidget(m_voiceButton);
    connect(m_voiceButton, &QPushButton::clicked, this, &MainWindow::toggleVoiceControl);
    statusBar()->addPermanentWidget(m_sysLabel);
    statusBar()->addPermanentWidget(m_clock);
    statusBar()->setSizeGripEnabled(false);

    auto *clockTimer = new QTimer(this);
    auto tick = [this, editionFooter, refreshEditionFooter] {
        m_clock->setText(QDateTime::currentDateTime().toString(QStringLiteral("dd.MM.yyyy  HH:mm:ss")));
        refreshEditionFooter();
    };
    connect(clockTimer, &QTimer::timeout, this, tick);

    // Загрузку CPU и памяти опрашиваем раз в 2 секунды (сам замер почти не нагружает систему)
    auto *sysTimer = new QTimer(this);
    sysTimer->setInterval(2000);
    connect(sysTimer, &QTimer::timeout, this, [this] {
        const SysStats s = readSysStats();
        if (s.cpuPercent < 0 || s.memPercent < 0) {
            m_sysLabel->setText(T("CPU / ОЗУ: недоступно"));
        } else {
            m_sysLabel->setText(T("CPU: %1%    ОЗУ: %2%  (%3 / %4 МБ)")
                                    .arg(qRound(s.cpuPercent)).arg(qRound(s.memPercent))
                                    .arg(s.memUsedMb).arg(s.memTotalMb));
        }
    });
    sysTimer->start();
    clockTimer->start(1000);
    tick();

    // ---- журнал событий и статус реагируют на события рекордеров ----
    connect(mgr, &CameraManager::cameraError, this, [this](const QString &id, const QString &msg) {
        const CameraEntry *e = m_mgr->find(id);
        const QString cameraName = e ? e->name : id;
        m_events->addEvent(cameraName, msg);
        writePsvmsLog(QStringLiteral("ОШИБКА КАМЕРЫ [%1]: %2").arg(cameraName, msg));
    });
    connect(mgr, &CameraManager::cameraStateChanged, this, [this](const QString &id, CameraWorker::State s) {
        if (s != CameraWorker::State::Connecting) {
            const CameraEntry *e = m_mgr->find(id);
            m_events->addEvent(e ? e->name : id, stateText(s));
        }
    });

    // ---- размер окна: сохранённый, иначе из настроек (в том числе для широкоформатных мониторов) ----
    QSettings s;
    if (!restoreGeometry(s.value(QStringLiteral("ui/geometry")).toByteArray())) {
        const int idx = qBound(0, s.value(QStringLiteral("ui/windowSize"), 0).toInt(),
                               int(sizeof(kMainWinSizes) / sizeof(kMainWinSizes[0])) - 1);
        if (idx > 0)
            resize(kMainWinSizes[idx].w, kMainWinSizes[idx].h);
        else
            resize(1500, 860);
    }

    // ---- горячие клавиши ----
    auto *fullscreenKey = new QShortcut(QKeySequence(Qt::Key_F11), this);
    connect(fullscreenKey, &QShortcut::activated, this, [this] {
        toggleMainFullscreen();
    });
    auto *escapeMainFullscreen = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    escapeMainFullscreen->setContext(Qt::WindowShortcut);
    connect(escapeMainFullscreen, &QShortcut::activated, this, [this] {
        if (isFullScreen())
            leaveMainFullscreen();
    });
    auto *refreshKey = new QShortcut(QKeySequence(Qt::Key_F5), this);
    connect(refreshKey, &QShortcut::activated, this, [this] {
        m_live->grid()->refreshCells();
        m_mgr->probeNow();
    });
    auto *closeCellKey = new QShortcut(QKeySequence(Qt::Key_Delete), m_live);
    closeCellKey->setContext(Qt::WidgetWithChildrenShortcut);   // не мешает Delete в текстовых полях
    connect(closeCellKey, &QShortcut::activated, this, [this] { m_live->grid()->closeSelectedCell(); });

    // 1 / 2 / 3 / 4 — быстрые раскладки 1, 2×2, 3×3, 4×4; действуют только когда открыт «Мониторинг»
    const int quickLayouts[4] = {1, 4, 9, 16};
    for (int i = 0; i < 4; ++i) {
        auto *key = new QShortcut(QKeySequence(QString::number(i + 1)), m_live);
        key->setContext(Qt::WidgetWithChildrenShortcut);
        const int count = quickLayouts[i];
        connect(key, &QShortcut::activated, this, [this, count] { m_live->grid()->setLayoutCount(count); });
    }

    // Единая мягкая анимация наведения и нажатия для кнопок всех основных разделов.
    auto *buttonMotion = new ButtonMotionFilter(this);
    for (QPushButton *button : findChildren<QPushButton *>())
        button->installEventFilter(buttonMotion);
    for (QToolButton *button : findChildren<QToolButton *>())
        button->installEventFilter(buttonMotion);

    // Автоблокировка выключена по умолчанию. При включении любое действие мышью/клавиатурой
    // сбрасывает таймер; по истечении времени возвращаем пользователя к экрану входа,
    // не завершая процесс PSVMS.
    m_inactivityTimer = new QTimer(this);
    m_inactivityTimer->setSingleShot(true);
    connect(m_inactivityTimer, &QTimer::timeout, this, [this] {
        SecurityAudit::write(QStringLiteral("session_locked_idle"));
        statusBar()->showMessage(T("Сеанс заблокирован из-за бездействия."), 3000);
        App::requestLogout();
    });
    qApp->installEventFilter(this);
    resetInactivityTimer();

}


void MainWindow::resetInactivityTimer()
{
    if (!m_inactivityTimer)
        return;
    const bool enabled = QSettings().value(QStringLiteral("security/idleLockEnabled"), false).toBool();
    if (!enabled) {
        m_inactivityTimer->stop();
        return;
    }
    const int minutes = qBound(1, QSettings().value(QStringLiteral("security/idleLockMinutes"), 10).toInt(), 240);
    m_inactivityTimer->start(minutes * 60 * 1000);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj)
    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::Wheel:
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TabletPress:
    case QEvent::TabletMove:
    case QEvent::User: // изменение настройки автоблокировки
        resetInactivityTimer();
        break;
    default:
        break;
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::toggleVoiceControl()
{
#ifdef Q_OS_WIN
    if (m_voiceProcess && m_voiceProcess->state() != QProcess::NotRunning) {
        stopVoiceControl();
        return;
    }

    if (!m_voiceProcess) {
        m_voiceProcess = new QProcess(this);
        m_voiceProcess->setProcessChannelMode(QProcess::MergedChannels);
        connect(m_voiceProcess, &QProcess::readyReadStandardOutput, this, [this] {
            m_voiceOutputBuffer += m_voiceProcess->readAllStandardOutput();
            while (true) {
                const int newline = m_voiceOutputBuffer.indexOf('\n');
                if (newline < 0) break;
                QByteArray line = m_voiceOutputBuffer.left(newline).trimmed();
                m_voiceOutputBuffer.remove(0, newline + 1);
                if (!line.isEmpty())
                    handleVoiceCommand(QString::fromUtf8(line));
            }
        });
        connect(m_voiceProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError) {
            if (m_voiceButton) {
                m_voiceButton->setChecked(false);
                m_voiceButton->setText(T("🎙 Голос"));
            }
            if (m_voiceProcess && m_voiceProcess->error() == QProcess::FailedToStart)
                statusBar()->showMessage(T("Не удалось запустить распознавание речи Windows"), 7000);
        });
        connect(m_voiceProcess, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
                [this](int, QProcess::ExitStatus) {
            if (m_voiceButton) {
                m_voiceButton->setChecked(false);
                m_voiceButton->setText(T("🎙 Голос"));
            }
        });
    }

    // Используется системный распознаватель речи Windows. Дополнительные DLL и Qt-модули не нужны.
    const QString script = QString::fromUtf8(R"PS(
$ErrorActionPreference = 'Stop'
[Console]::OutputEncoding = New-Object System.Text.UTF8Encoding($false)
try {
  Add-Type -AssemblyName System.Speech
  $culture = [System.Globalization.CultureInfo]::GetCultureInfo('ru-RU')
  $recognizer = [System.Speech.Recognition.SpeechRecognitionEngine]::new($culture)
  $recognizer.SetInputToDefaultAudioDevice()
  $choices = [System.Speech.Recognition.Choices]::new()
  @(
    'главная','открой главную','покажи камеры','открой камеры','мониторинг',
    'устройства','открой устройства','события','открой события','настройки','открой настройки',
    'состояние камер','покажи состояние камер','покажи камеры не в сети','камеры не в сети','не в сети',
    'покажи все камеры','все камеры','переключи на четыре камеры','переключи на 4 камеры',
    'переключи на девять камер','переключи на 9 камер','переключи на шестнадцать камер','переключи на 16 камер',
    'полный экран','включи полный экран','включи полноэкранный режим',
    'выйди из полного экрана','выключи полный экран','обнови камеры','обновить камеры',
    'раскладка один','раскладка четыре','раскладка девять','раскладка шестнадцать',
    'одна камера','четыре камеры','девять камер','шестнадцать камер',
    'стоп','остановить голосовое управление'
  ) | ForEach-Object { [void]$choices.Add($_) }
  $grammar = [System.Speech.Recognition.Grammar]::new($choices)
  $recognizer.LoadGrammar($grammar)
  $recognizer.InitialSilenceTimeout = [TimeSpan]::FromSeconds(3)
  $recognizer.BabbleTimeout = [TimeSpan]::FromSeconds(2)
  while ($true) {
    $result = $recognizer.Recognize()
    if ($null -ne $result -and $null -ne $result.Text) {
      [Console]::WriteLine($result.Text)
      [Console]::Out.Flush()
      if ($result.Text -eq 'стоп' -or $result.Text -eq 'остановить голосовое управление') { break }
    }
  }
  $recognizer.Dispose()
} catch {
  [Console]::Error.WriteLine($_.Exception.Message)
  exit 2
}
)PS");
    m_voiceOutputBuffer.clear();
    m_voiceProcess->start(QStringLiteral("powershell.exe"),
                          {QStringLiteral("-NoProfile"), QStringLiteral("-ExecutionPolicy"),
                           QStringLiteral("Bypass"), QStringLiteral("-Command"), script});
    if (m_voiceButton) {
        m_voiceButton->setChecked(true);
        m_voiceButton->setText(T("🔴 Слушаю"));
    }
    statusBar()->showMessage(T("Голосовое управление включено. Скажите, например: «покажи камеры» или «полный экран»"), 9000);
#else
    QMessageBox::information(this, T("Голосовое управление"),
                             T("Голосовое управление в этой версии доступно в Windows."));
    if (m_voiceButton) m_voiceButton->setChecked(false);
#endif
}

void MainWindow::stopVoiceControl()
{
    if (m_voiceProcess && m_voiceProcess->state() != QProcess::NotRunning) {
        m_voiceProcess->terminate();
        if (!m_voiceProcess->waitForFinished(700))
            m_voiceProcess->kill();
    }
    if (m_voiceButton) {
        m_voiceButton->setChecked(false);
        m_voiceButton->setText(T("🎙 Голос"));
    }
    statusBar()->showMessage(T("Голосовое управление выключено"), 4000);
}

void MainWindow::handleVoiceCommand(const QString &rawCommand)
{
    QString command = rawCommand.trimmed().toLower();
    command.replace(QChar(0x0451), QChar(0x0435)); // ё -> е
    command.replace(QChar(0x00A0), QChar(' '));
    while (command.contains(QStringLiteral("  ")))
        command.replace(QStringLiteral("  "), QStringLiteral(" "));
    statusBar()->showMessage(T("Распознано: %1").arg(rawCommand), 3500);

    if (command == QStringLiteral("стоп") || command == QStringLiteral("остановить голосовое управление")) {
        stopVoiceControl();
    } else if (command == QStringLiteral("покажи все камеры") || command == QStringLiteral("все камеры") ||
               command == QStringLiteral("покажи камеры") || command == QStringLiteral("открой камеры") ||
               command == QStringLiteral("главная") || command == QStringLiteral("открой главную") ||
               command == QStringLiteral("мониторинг")) {
        const int total = int(m_mgr->cameras().size());
        if (total <= 0) {
            m_nav->setCurrent(PageLive);
            statusBar()->showMessage(T("Список камер пуст"), 5000);
            return;
        }
        int layout = 64;
        const int supported[] = {1, 4, 6, 8, 9, 12, 16, 20, 25, 36, 49, 64};
        for (int n : supported) {
            if (n >= total) { layout = n; break; }
        }
        if (total > 64) {
            statusBar()->showMessage(T("В раскладке можно одновременно показать не более 64 камер"), 6000);
            layout = 64;
        }
        m_nav->setCurrent(PageLive);
        m_live->grid()->clearAll();
        m_live->grid()->setLayoutCount(layout);
        const int shown = qMin(total, layout);
        for (int i = 0; i < shown; ++i)
            m_live->grid()->openCamera(m_mgr->cameras().at(i).id);
        statusBar()->showMessage(T("Показано камер: %1 из %2").arg(shown).arg(total), 5000);
    } else if (command == QStringLiteral("устройства") || command == QStringLiteral("открой устройства")) {
        m_nav->setCurrent(PageDevices);
    } else if (command == QStringLiteral("события") || command == QStringLiteral("открой события")) {
        if (isProVersion()) m_nav->setCurrent(PageEvents);
        else statusBar()->showMessage(T("Раздел событий недоступен в этой редакции"), 4000);
    } else if (command == QStringLiteral("настройки") || command == QStringLiteral("открой настройки")) {
        if (Users::current().isAdmin()) {
            m_stack->setCurrentIndex(PageSettings);
            m_top->setTitle(pageTitle(PageSettings));
        } else statusBar()->showMessage(T("Настройки доступны только администратору"), 4000);
    } else if (command == QStringLiteral("состояние камер") || command == QStringLiteral("покажи состояние камер")) {
        if (m_cameraStatusDialog) {
            m_cameraStatusDialog->showAllCameras();
        } else {
            m_cameraStatusDialog = new CameraStatusDialog(m_mgr, this);
            m_cameraStatusDialog->show();
            m_cameraStatusDialog->raise();
            m_cameraStatusDialog->activateWindow();
        }
    } else if (command == QStringLiteral("покажи камеры не в сети") ||
               command == QStringLiteral("камеры не в сети") || command == QStringLiteral("не в сети")) {
        if (!m_cameraStatusDialog)
            m_cameraStatusDialog = new CameraStatusDialog(m_mgr, this);
        m_cameraStatusDialog->showOfflineOnly();
    } else if (command == QStringLiteral("полный экран") || command == QStringLiteral("включи полный экран") ||
               command == QStringLiteral("включи полноэкранный режим")) {
        if (!isFullScreen()) toggleMainFullscreen();
    } else if (command == QStringLiteral("выйди из полного экрана") || command == QStringLiteral("выключи полный экран")) {
        if (isFullScreen()) leaveMainFullscreen();
    } else if (command == QStringLiteral("обнови камеры") || command == QStringLiteral("обновить камеры")) {
        m_live->grid()->refreshCells();
        m_mgr->probeNow();
    } else if (command == QStringLiteral("раскладка один") || command == QStringLiteral("одна камера")) {
        m_nav->setCurrent(PageLive); m_live->grid()->setLayoutCount(1);
    } else if (command == QStringLiteral("раскладка четыре") || command == QStringLiteral("четыре камеры") ||
               command == QStringLiteral("переключи на четыре камеры") || command == QStringLiteral("переключи на 4 камеры")) {
        m_nav->setCurrent(PageLive); m_live->grid()->setLayoutCount(4);
    } else if (command == QStringLiteral("раскладка девять") || command == QStringLiteral("девять камер") ||
               command == QStringLiteral("переключи на девять камер") || command == QStringLiteral("переключи на 9 камер")) {
        m_nav->setCurrent(PageLive); m_live->grid()->setLayoutCount(9);
    } else if (command == QStringLiteral("раскладка шестнадцать") || command == QStringLiteral("шестнадцать камер") ||
               command == QStringLiteral("переключи на шестнадцать камер") || command == QStringLiteral("переключи на 16 камер")) {
        m_nav->setCurrent(PageLive); m_live->grid()->setLayoutCount(16);
    }
}

void MainWindow::toggleMainFullscreen()
{
    if (isFullScreen()) {
        leaveMainFullscreen();
        return;
    }

    // Запоминаем состояние именно перед первым входом в fullscreen.
    // Это позволяет вернуться к исходному размеру/положению, а не к размеру Qt по умолчанию.
    m_preFullscreenMaximized = isMaximized();
    m_preFullscreenGeometry = geometry();
    m_hasPreFullscreenState = true;
    showFullScreen();
}

void MainWindow::leaveMainFullscreen()
{
    if (!isFullScreen())
        return;

    // Выход из полноэкранного режима всегда возвращает главное окно
    // в развёрнутое состояние, независимо от прежней геометрии окна.
    showMaximized();
    m_hasPreFullscreenState = false;
}

bool MainWindow::startFullscreen() const
{
    return QSettings().value(QStringLiteral("ui/startFullscreen"), false).toBool();
}

QString MainWindow::pageTitle(int page) const
{
    switch (page) {
    case PageLive:     return T("Главная");
    case PageDevices:  return T("Камеры");
    case PageEvents:   return T("События");
    case PageSettings: return T("Настройки");
    }
    return QString();
}

void MainWindow::updateLanguageItem()
{
    m_nav->setItemText(m_langItem, QStringLiteral("🌐"), T("Язык") + QStringLiteral(": ") + I18n::code().toUpper());
}

void MainWindow::showLanguageMenu()
{
    QMenu menu(this);
    auto *group = new QActionGroup(&menu);
    const QString names[2] = {QString::fromUtf8("Русский"), QStringLiteral("English")};
    const QString codes[2] = {QStringLiteral("ru"), QStringLiteral("en")};
    for (int i = 0; i < 2; ++i) {
        QAction *a = menu.addAction(names[i]);
        a->setCheckable(true);
        a->setChecked(codes[i] == I18n::code());
        a->setData(codes[i]);
        group->addAction(a);
    }
    QWidget *b = m_nav->itemButton(m_langItem);
    const QPoint pos = b ? b->mapToGlobal(QPoint(b->width() + 4, 0)) : QCursor::pos();
    QAction *chosen = menu.exec(pos);
    if (chosen && chosen->data().toString() != I18n::code())
        applyLanguage(chosen->data().toString());
}

void MainWindow::applyLanguage(const QString &code)
{
    I18n::setLanguageByCode(code);
    QSettings s;
    s.setValue(QStringLiteral("ui/geometry"), saveGeometry());
    s.setValue(QStringLiteral("ui/navExpanded"), m_nav->isExpanded());
    m_live->grid()->flushLayout();
    I18n::requestRestart();   // окно пересоздаётся на новом языке, запись камер не прерывается
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    stopVoiceControl();
    // При выходе пароль разработчика не запрашивается.
    // Он требуется только при запуске; неверный пароль там приводит к повторному запросу.
    m_live->grid()->flushLayout();
    QSettings s;
    s.setValue(QStringLiteral("ui/geometry"), saveGeometry());
    s.setValue(QStringLiteral("ui/navExpanded"), m_nav->isExpanded());
    m_mgr->shutdown();   // корректно закрыть текущие сегменты записи
    QMainWindow::closeEvent(e);
}

// ======================================================================
// app/onvif_import_dialog.cpp
// ======================================================================
namespace {

enum Column_onvifdlg { ColCheck_onvifdlg, ColAddress_onvifdlg, ColName_onvifdlg, ColState_onvifdlg, ColCount_onvifdlg };

constexpr int kMaxRange_onvifdlg = 2048;

bool parseIpv4_onvifdlg(const QString &s, quint32 *out)
{
    QHostAddress a;
    if (!a.setAddress(s.trimmed()) || a.protocol() != QAbstractSocket::IPv4Protocol)
        return false;
    *out = a.toIPv4Address();
    return true;
}

} // namespace

OnvifImportDialog::OnvifImportDialog(CameraManager *mgr, QWidget *parent) : QDialog(parent), m_mgr(mgr)
{
    setWindowTitle(T("Импорт камер по ONVIF"));
    resize(940, 640);

    for (const CameraEntry &e : m_mgr->cameras())
        m_existingHosts.insert(QUrl(e.url).host());

    m_discovery = new OnvifDiscovery(this);
    m_scanner = new OnvifScanner(this);
    m_resolver = new OnvifResolver(this);

    // ---- поиск ----
    auto *findBox = new QGroupBox(T("1. Поиск камер"), this);
    auto *findLay = new QHBoxLayout(findBox);
    m_btnDiscover = new QPushButton(T("Найти в своей сети"), findBox);
    m_from = new QLineEdit(findBox);
    m_from->setPlaceholderText(QStringLiteral("192.168.1.1"));
    m_to = new QLineEdit(findBox);
    m_to->setPlaceholderText(QStringLiteral("192.168.1.254"));
    m_port = new QSpinBox(findBox);
    m_port->setRange(1, 65535);
    m_port->setValue(80);
    m_port->setToolTip(T("Порт ONVIF: обычно 80; у камер Xiongmai/iCSee — 8899"));
    m_btnScan = new QPushButton(T("Сканировать диапазон"), findBox);
    findLay->addWidget(m_btnDiscover);
    findLay->addSpacing(20);
    findLay->addWidget(new QLabel(T("Диапазон IP:"), findBox));
    findLay->addWidget(m_from);
    findLay->addWidget(new QLabel(QStringLiteral("—"), findBox));
    findLay->addWidget(m_to);
    findLay->addWidget(new QLabel(T("порт"), findBox));
    findLay->addWidget(m_port);
    findLay->addWidget(m_btnScan);

    // ---- доступ ----
    auto *authBox = new QGroupBox(T("2. Общие логин и пароль камер"), this);
    auto *authLay = new QHBoxLayout(authBox);
    m_user = new QLineEdit(QStringLiteral("admin"), authBox);
    m_pass = new QLineEdit(authBox);
    m_pass->setEchoMode(QLineEdit::Password);
    m_group = new QLineEdit(authBox);
    m_group->setPlaceholderText(T("например, Склад"));
    authLay->addWidget(new QLabel(T("Логин:"), authBox));
    authLay->addWidget(m_user);
    authLay->addWidget(new QLabel(T("Пароль:"), authBox));
    authLay->addWidget(m_pass);
    authLay->addWidget(new QLabel(T("Группа:"), authBox));
    authLay->addWidget(m_group);

    // ---- таблица ----
    m_table = new QTableWidget(0, ColCount_onvifdlg, this);
    m_table->setHorizontalHeaderLabels({QString(), T("Адрес"),
                                        T("Имя / модель"), T("Состояние")});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->verticalHeader()->setVisible(false);
    m_table->setColumnWidth(ColCheck_onvifdlg, 34);
    m_table->setColumnWidth(ColAddress_onvifdlg, 150);
    m_table->setColumnWidth(ColName_onvifdlg, 300);
    m_table->horizontalHeader()->setSectionResizeMode(ColState_onvifdlg, QHeaderView::Stretch);

    auto *selAll = new QPushButton(T("Выбрать все"), this);
    auto *selNone = new QPushButton(T("Снять выбор"), this);
    auto *selRow = new QHBoxLayout;
    selRow->addWidget(new QLabel(T("3. Найденные устройства"), this));
    selRow->addStretch(1);
    selRow->addWidget(selAll);
    selRow->addWidget(selNone);

    // ---- низ ----
    m_progress = new QProgressBar(this);
    m_progress->setRange(0, 1);
    m_progress->setValue(0);
    m_status = new QLabel(T("Нажмите «Найти в своей сети» или задайте диапазон IP."), this);
    m_btnImport = new QPushButton(T("Добавить выбранные"), this);
    auto *close = new QPushButton(T("Закрыть"), this);

    auto *bottom = new QHBoxLayout;
    bottom->addWidget(m_progress, 1);
    bottom->addWidget(m_btnImport);
    bottom->addWidget(close);

    auto *root = new QVBoxLayout(this);
    root->addWidget(findBox);
    root->addWidget(authBox);
    root->addLayout(selRow);
    root->addWidget(m_table, 1);
    root->addWidget(m_status);
    root->addLayout(bottom);

    // ---- сигналы ----
    connect(m_btnDiscover, &QPushButton::clicked, this, &OnvifImportDialog::startDiscovery);
    connect(m_btnScan, &QPushButton::clicked, this, &OnvifImportDialog::startScan);
    connect(m_btnImport, &QPushButton::clicked, this, &OnvifImportDialog::importSelected);
    connect(selAll, &QPushButton::clicked, this, [this] { setAllChecked(true); });
    connect(selNone, &QPushButton::clicked, this, [this] { setAllChecked(false); });
    connect(close, &QPushButton::clicked, this, &QDialog::accept);

    connect(m_discovery, &OnvifDiscovery::deviceFound, this, &OnvifImportDialog::addDevice);
    connect(m_discovery, &OnvifDiscovery::finished, this, [this] {
        m_progress->setRange(0, 1);
        m_progress->setValue(1);
        m_status->setText(T("Поиск завершён. Найдено устройств: %1").arg(m_devices.size()));
        setBusy(false);
    });

    connect(m_scanner, &OnvifScanner::deviceFound, this, &OnvifImportDialog::addDevice);
    connect(m_scanner, &OnvifScanner::progress, this, [this](int done, int total) {
        m_progress->setRange(0, total);
        m_progress->setValue(done);
        m_status->setText(T("Сканирование: %1 из %2, найдено: %3")
                              .arg(done).arg(total).arg(m_devices.size()));
    });
    connect(m_scanner, &OnvifScanner::finished, this, [this] {
        m_status->setText(T("Сканирование завершено. Найдено устройств: %1").arg(m_devices.size()));
        setBusy(false);
    });

    connect(m_resolver, &OnvifResolver::progress, this, [this](int done, int total) {
        m_progress->setRange(0, total);
        m_progress->setValue(done);
        m_status->setText(T("Опрос камер: %1 из %2").arg(done).arg(total));
    });
    connect(m_resolver, &OnvifResolver::resolved, this, [this](int i, const OnvifStreams &s) {
        const int row = m_importRows.at(i);
        if (!s.ok) {
            ++m_failed;
            m_table->item(row, ColState_onvifdlg)->setText(s.error);
            return;
        }
        CameraEntry e;
        e.name = displayName(row);
        e.group = m_group->text().trimmed();
        e.url = s.mainUrl;
        e.liveUrl = s.subUrl;
        e.record = true;
        if (s.hasPtz) {
            e.ptzXAddr = s.ptzXAddr;
            e.ptzToken = s.ptzToken;
            e.ptzUser = m_user->text().trimmed();
            e.ptzPassword = m_pass->text();
        }
        m_pending.append(e);
        m_pendingRows.append(row);
        QString status = s.subUrl.isEmpty() ? T("Получен 1 поток") : T("Получены основной поток и подпоток");
        if (s.hasPtz)
            status += QStringLiteral("  ·  ") + T("PTZ поддерживается");
        m_table->item(row, ColState_onvifdlg)->setText(status);
    });
    connect(m_resolver, &OnvifResolver::finished, this, [this] {
        if (!m_pending.isEmpty())
            m_mgr->addMany(m_pending);   // одно сохранение и один пересбор интерфейса на всех
        for (int row : std::as_const(m_pendingRows)) {
            m_table->item(row, ColState_onvifdlg)->setText(T("Добавлена"));
            m_table->item(row, ColCheck_onvifdlg)->setCheckState(Qt::Unchecked);
            m_table->item(row, ColCheck_onvifdlg)->setFlags(Qt::NoItemFlags);
            m_existingHosts.insert(m_devices.at(row).address);
        }
        m_status->setText(T("Добавлено камер: %1, с ошибкой: %2%3")
                              .arg(m_pending.size()).arg(m_failed)
                              .arg(m_failed ? T(" (исправьте логин/пароль и повторите)") : QString()));
        m_pending.clear();
        m_pendingRows.clear();
        setBusy(false);
    });
}

QString OnvifImportDialog::displayName(int row) const
{
    const OnvifDevice &d = m_devices.at(row);
    QString base = d.name.isEmpty() ? d.hardware : d.name;
    return base.isEmpty() ? d.address : QStringLiteral("%1 (%2)").arg(base, d.address);
}

void OnvifImportDialog::addDevice(const OnvifDevice &dev)
{
    for (const OnvifDevice &d : std::as_const(m_devices))
        if (d.address == dev.address)
            return;

    const int row = m_table->rowCount();
    m_table->insertRow(row);
    m_devices.append(dev);

    const bool exists = m_existingHosts.contains(dev.address);

    auto *chk = new QTableWidgetItem;
    chk->setFlags(exists ? Qt::NoItemFlags : (Qt::ItemIsUserCheckable | Qt::ItemIsEnabled));
    chk->setCheckState(exists ? Qt::Unchecked : Qt::Checked);
    m_table->setItem(row, ColCheck_onvifdlg, chk);
    m_table->setItem(row, ColAddress_onvifdlg, new QTableWidgetItem(dev.address));

    QString title = dev.name;
    if (!dev.hardware.isEmpty())
        title += title.isEmpty() ? dev.hardware : QStringLiteral(" (%1)").arg(dev.hardware);
    m_table->setItem(row, ColName_onvifdlg, new QTableWidgetItem(title));
    m_table->setItem(row, ColState_onvifdlg,
                     new QTableWidgetItem(exists ? T("Уже добавлена") : T("Найдена")));
}

void OnvifImportDialog::setAllChecked(bool on)
{
    for (int row = 0; row < m_table->rowCount(); ++row) {
        QTableWidgetItem *it = m_table->item(row, ColCheck_onvifdlg);
        if (it->flags() & Qt::ItemIsUserCheckable)
            it->setCheckState(on ? Qt::Checked : Qt::Unchecked);
    }
}

void OnvifImportDialog::setBusy(bool busy)
{
    m_btnDiscover->setEnabled(!busy);
    m_btnScan->setEnabled(!busy);
    m_btnImport->setEnabled(!busy);
}

void OnvifImportDialog::startDiscovery()
{
    setBusy(true);
    m_progress->setRange(0, 0);   // «бегущая» полоса
    m_status->setText(T("Поиск камер в сети…"));
    m_discovery->start(4000);
}

void OnvifImportDialog::startScan()
{
    quint32 a = 0, b = 0;
    if (!parseIpv4_onvifdlg(m_from->text(), &a) || !parseIpv4_onvifdlg(m_to->text(), &b) || b < a) {
        QMessageBox::warning(this, windowTitle(), T("Укажите корректный диапазон IPv4-адресов."));
        return;
    }
    if (quint64(b) - a + 1 > kMaxRange_onvifdlg) {
        QMessageBox::warning(this, windowTitle(),
            T("Слишком большой диапазон: максимум %1 адресов за раз.").arg(kMaxRange_onvifdlg));
        return;
    }
    setBusy(true);
    m_progress->setRange(0, int(b - a + 1));
    m_progress->setValue(0);
    m_scanner->start(a, b, quint16(m_port->value()), 64);
}

void OnvifImportDialog::importSelected()
{
    if (m_user->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, windowTitle(), T("Укажите логин камер."));
        return;
    }

    m_importRows.clear();
    m_pending.clear();
    m_pendingRows.clear();
    m_failed = 0;

    QList<OnvifRequest> requests;
    for (int row = 0; row < m_table->rowCount(); ++row) {
        const QTableWidgetItem *chk = m_table->item(row, ColCheck_onvifdlg);
        if (!(chk->flags() & Qt::ItemIsUserCheckable) || chk->checkState() != Qt::Checked)
            continue;
        requests.append({m_devices.at(row).xaddr, m_user->text().trimmed(), m_pass->text()});
        m_importRows.append(row);
        m_table->item(row, ColState_onvifdlg)->setText(T("Запрос…"));
    }
    if (requests.isEmpty()) {
        QMessageBox::information(this, windowTitle(), T("Отметьте камеры для добавления."));
        return;
    }

    setBusy(true);
    m_progress->setRange(0, int(requests.size()));
    m_progress->setValue(0);
    m_resolver->start(requests, 8);   // одновременно опрашиваем 8 камер
}

// ======================================================================
// app/playback_page.cpp
// ======================================================================
namespace {

QString formatMs(qint64 ms)
{
    const qint64 s = ms / 1000;
    return QStringLiteral("%1:%2").arg(s / 60, 2, 10, QLatin1Char('0')).arg(s % 60, 2, 10, QLatin1Char('0'));
}

} // namespace

PlaybackPage::PlaybackPage(CameraManager *mgr, QWidget *parent) : QWidget(parent), m_mgr(mgr)
{
    m_index.open(mgr->dbPath());

    // ---- левая колонка: простой поиск и навигация по архиву ----
    auto *archiveTitle = new QLabel(T("АРХИВ ЗАПИСЕЙ"), this);
    archiveTitle->setStyleSheet(QStringLiteral(
        "QLabel { color:#f6d34d; background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #3b4249,stop:1 #22272d); "
        "border:1px solid #8e741b; border-radius:7px; padding:11px 12px; font-size:15px; font-weight:800; }"));
    auto *archiveHint = new QLabel(T("Выберите камеру и дату, затем откройте нужную запись."), this);
    archiveHint->setWordWrap(true);
    archiveHint->setStyleSheet(QStringLiteral("color:#8995a5; font-size:11px; padding:2px 3px;"));

    m_camera = new QComboBox(this);
    m_camera->setMinimumHeight(34);
    m_calendar = new QCalendarWidget(this);
    m_calendar->setGridVisible(true);
    m_calendar->setMaximumDate(QDate::currentDate());
    m_calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    m_list = new QListWidget(this);
    m_list->setAlternatingRowColors(true);
    m_list->setSpacing(3);
    m_search = new QLineEdit(this);
    m_search->setPlaceholderText(T("Поиск записи по времени…"));
    m_archiveCount = new QLabel(this);
    m_archiveCount->setStyleSheet(QStringLiteral("color:#7d8998; font-size:11px;"));

    auto *quickDates = new QHBoxLayout;
    auto *todayBtn = new QPushButton(T("Сегодня"), this);
    auto *yesterdayBtn = new QPushButton(T("Вчера"), this);
    todayBtn->setToolTip(T("Показать записи за сегодня"));
    yesterdayBtn->setToolTip(T("Показать записи за вчера"));
    quickDates->addWidget(todayBtn);
    quickDates->addWidget(yesterdayBtn);
    auto *refresh = new QPushButton(T("⟳  Обновить архив"), this);
    auto *exportBtn = new QPushButton(T("⇩  Экспортировать выбранную запись"), this);
    exportBtn->setObjectName(QStringLiteral("archiveExportButton"));
    exportBtn->setToolTip(T("Сохранить выбранный сегмент архива в отдельный MKV-файл"));

    auto *left = new QVBoxLayout;
    left->setSpacing(8);
    left->addWidget(archiveTitle);
    left->addWidget(archiveHint);
    left->addWidget(new QLabel(T("Камера"), this));
    left->addWidget(m_camera);
    left->addLayout(quickDates);
    left->addWidget(m_calendar);
    left->addWidget(new QLabel(T("Записи за выбранный день"), this));
    left->addWidget(m_search);
    left->addWidget(m_archiveCount);
    left->addWidget(m_list, 1);
    left->addWidget(refresh);
    left->addWidget(exportBtn);

    auto *leftBox = new QWidget(this);
    leftBox->setLayout(left);
    leftBox->setFixedWidth(360);
    leftBox->setStyleSheet(QStringLiteral(
        "QListWidget { border:1px solid #59616a; border-radius:7px; padding:4px; "
        "background:#1a1e23; alternate-background-color:#22272d; }"
        "QListWidget::item { padding:8px 7px; border-radius:5px; border-bottom:1px solid #343a41; }"
        "QListWidget::item:hover { background:#343b42; }"
        "QListWidget::item:selected { background:#e9bd00; color:#171a1e; }"
        "QPushButton { min-height:30px; }"));

    connect(todayBtn, &QPushButton::clicked, this, [this] {
        m_calendar->setSelectedDate(QDate::currentDate());
    });
    connect(yesterdayBtn, &QPushButton::clicked, this, [this] {
        m_calendar->setSelectedDate(QDate::currentDate().addDays(-1));
    });
    connect(m_search, &QLineEdit::textChanged, this, [this](const QString &query) {
        int visible = 0;
        for (int i = 0; i < m_list->count(); ++i) {
            QListWidgetItem *item = m_list->item(i);
            const bool show = item->data(Qt::UserRole).isValid()
                && item->text().contains(query.trimmed(), Qt::CaseInsensitive);
            item->setHidden(!show);
            if (show) ++visible;
        }
        if (m_archiveCount)
            m_archiveCount->setText(T("Найдено: %1").arg(visible));
    });

    // ---- правая часть: видео и управление ----
    m_video = new QVideoWidget(this);
    m_video->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_player = new QMediaPlayer(this);
    m_audio = new QAudioOutput(this);
    m_player->setAudioOutput(m_audio);
    m_player->setVideoOutput(m_video);

    m_playBtn = new QPushButton(QStringLiteral("▶"), this);
    m_playBtn->setFixedWidth(48);
    m_slider = new QSlider(Qt::Horizontal, this);
    m_time = new QLabel(QStringLiteral("00:00 / 00:00"), this);

    auto *controls = new QHBoxLayout;
    controls->addWidget(m_playBtn);
    controls->addWidget(m_slider, 1);
    controls->addWidget(m_time);

    auto *right = new QVBoxLayout;
    right->addWidget(m_video, 1);
    right->addLayout(controls);

    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->addWidget(leftBox);
    root->addLayout(right, 1);

    // ---- сигналы ----
    connect(m_mgr, &CameraManager::camerasChanged, this, &PlaybackPage::reloadCameras);
    connect(m_camera, &QComboBox::currentIndexChanged, this, &PlaybackPage::reloadSegments);
    connect(m_calendar, &QCalendarWidget::selectionChanged, this, &PlaybackPage::reloadSegments);
    connect(refresh, &QPushButton::clicked, this, &PlaybackPage::reloadSegments);
    connect(exportBtn, &QPushButton::clicked, this, [this] {
        const QListWidgetItem *item = m_list->currentItem();
        if (!item || !item->data(Qt::UserRole).isValid()) {
            QMessageBox::information(this, T("Экспорт архива"), T("Сначала выберите запись в списке."));
            return;
        }
        const QString sourcePath = item->data(Qt::UserRole).toString();
        if (!QFileInfo::exists(sourcePath)) {
            QMessageBox::warning(this, T("Экспорт архива"), T("Файл записи не найден на диске."));
            reloadSegments();
            return;
        }
        QString suggested = QFileInfo(sourcePath).completeBaseName() + QStringLiteral(".mkv");
        const QString targetPath = QFileDialog::getSaveFileName(this, T("Экспортировать запись"),
            QDir::toNativeSeparators(QDir::home().filePath(suggested)),
            T("Видео Matroska (*.mkv);;Все файлы (*.*)"));
        if (targetPath.isEmpty())
            return;
        QFile input(sourcePath);
        if (!input.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, T("Экспорт архива"), T("Не удалось открыть запись: %1").arg(input.errorString()));
            return;
        }
        QSaveFile output(targetPath);
        if (!output.open(QIODevice::WriteOnly)) {
            QMessageBox::warning(this, T("Экспорт архива"), T("Не удалось создать файл: %1").arg(output.errorString()));
            return;
        }
        QByteArray buffer(1024 * 1024, Qt::Uninitialized);
        while (!input.atEnd()) {
            const qint64 bytesRead = input.read(buffer.data(), buffer.size());
            if (bytesRead < 0 || output.write(buffer.constData(), bytesRead) != bytesRead) {
                output.cancelWriting();
                QMessageBox::warning(this, T("Экспорт архива"), T("Ошибка чтения или записи при экспорте."));
                return;
            }
        }
        if (!output.commit()) {
            QMessageBox::warning(this, T("Экспорт архива"), T("Не удалось завершить экспорт: %1").arg(output.errorString()));
            return;
        }
        QMessageBox::information(this, T("Экспорт архива"), T("Запись успешно экспортирована."));
    });
    connect(m_list, &QListWidget::itemDoubleClicked, this, &PlaybackPage::playCurrent);
    connect(m_playBtn, &QPushButton::clicked, this, &PlaybackPage::togglePlay);

    connect(m_player, &QMediaPlayer::durationChanged, this, [this](qint64 d) {
        m_slider->setRange(0, int(d));
    });
    connect(m_player, &QMediaPlayer::positionChanged, this, [this](qint64 p) {
        if (!m_slider->isSliderDown())
            m_slider->setValue(int(p));
        m_time->setText(formatMs(p) + QStringLiteral(" / ") + formatMs(m_player->duration()));
    });
    connect(m_slider, &QSlider::sliderMoved, m_player, &QMediaPlayer::setPosition);
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState s) {
        m_playBtn->setText(s == QMediaPlayer::PlayingState ? QStringLiteral("⏸") : QStringLiteral("▶"));
    });

    reloadCameras();
}

void PlaybackPage::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    m_calendar->setMaximumDate(QDate::currentDate());
    reloadSegments();   // подхватить сегменты, записанные пока страница была скрыта
}

void PlaybackPage::reloadCameras()
{
    const QString keep = m_camera->currentData().toString();

    m_camera->blockSignals(true);
    m_camera->clear();
    for (const CameraEntry &e : m_mgr->cameras())
        m_camera->addItem(e.name, e.id);
    const int idx = m_camera->findData(keep);
    if (idx >= 0)
        m_camera->setCurrentIndex(idx);
    m_camera->blockSignals(false);

    reloadSegments();
}

void PlaybackPage::reloadSegments()
{
    m_list->clear();
    const QString camId = m_camera->currentData().toString();
    if (camId.isEmpty()) {
        m_archiveCount->setText(T("Добавьте камеру, чтобы просматривать архив."));
        auto *empty = new QListWidgetItem(T("Нет выбранной камеры"), m_list);
        empty->setFlags(Qt::NoItemFlags);
        return;
    }

    const QDate day = m_calendar->selectedDate();
    const qint64 from = day.startOfDay().toMSecsSinceEpoch();
    const qint64 to = day.addDays(1).startOfDay().toMSecsSinceEpoch();
    const QList<SegmentInfo> segments = m_index.segmentsInRange(camId, from, to);

    for (const SegmentInfo &s : segments) {
        QString text = QDateTime::fromMSecsSinceEpoch(s.startMs).toString(QStringLiteral("HH:mm:ss"))
                       + QStringLiteral("  —  ");
        text += s.endMs ? QDateTime::fromMSecsSinceEpoch(s.endMs).toString(QStringLiteral("HH:mm:ss"))
                        : T("идёт запись");
        if (s.sizeBytes > 0)
            text += T("   ·   %1 МБ").arg(s.sizeBytes / 1048576.0, 0, 'f', 1);

        auto *it = new QListWidgetItem(QStringLiteral("▶  ") + text, m_list);
        it->setData(Qt::UserRole, s.path);
        it->setToolTip(T("Дважды щёлкните, чтобы воспроизвести запись"));
    }

    if (segments.isEmpty()) {
        auto *empty = new QListWidgetItem(T("За этот день записей нет.\nВыберите другую дату или нажмите «Обновить архив»."), m_list);
        empty->setFlags(Qt::NoItemFlags);
        empty->setTextAlignment(Qt::AlignCenter);
    }
    m_archiveCount->setText(T("Записей: %1").arg(segments.size()));
    const QString query = m_search->text().trimmed();
    if (!query.isEmpty()) {
        for (int i = 0; i < m_list->count(); ++i) {
            QListWidgetItem *item = m_list->item(i);
            item->setHidden(!item->data(Qt::UserRole).isValid()
                            || !item->text().contains(query, Qt::CaseInsensitive));
        }
    }
}

void PlaybackPage::playCurrent()
{
    const QListWidgetItem *it = m_list->currentItem();
    if (!it)
        return;
    const QString path = it->data(Qt::UserRole).toString();
    if (!QFileInfo::exists(path))
        return;
    m_player->setSource(QUrl::fromLocalFile(path));
    m_player->play();
}

void PlaybackPage::togglePlay()
{
    if (m_player->playbackState() == QMediaPlayer::PlayingState)
        m_player->pause();
    else
        m_player->play();
}

void PlaybackPage::selectCamera(const QString &id)
{
    const int idx = m_camera->findData(id);
    if (idx >= 0)
        m_camera->setCurrentIndex(idx);
}

// ======================================================================
// app/playback_window.cpp
// ======================================================================
PlaybackWindow::PlaybackWindow(CameraManager *mgr, const QString &cameraId, QWidget *parent) : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(T("Архив"));
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    resize(1180, 700);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    auto *page = new PlaybackPage(mgr, this);
    lay->addWidget(page);
    if (!cameraId.isEmpty())
        page->selectCamera(cameraId);
}

// ======================================================================
// app/ptz_panel.cpp
// ======================================================================
namespace {
// Направления по индексу кнопки 0..8 (как на цифровой клавиатуре, без центра): x, y
const double kDirX[9] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
const double kDirY[9] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
}

PtzPanel::PtzPanel(CameraManager *mgr, QWidget *parent) : QWidget(parent), m_mgr(mgr)
{
    m_nam = new QNetworkAccessManager(this);

    auto *box = new QGroupBox(T("PTZ-управление"), this);
    auto *lay = new QGridLayout(box);

    const char *arrows[9] = {"↖", "↑", "↗", "←", "●", "→", "↙", "↓", "↘"};
    for (int i = 0; i < 9; ++i) {
        auto *b = new QToolButton(box);
        b->setText(QString::fromUtf8(arrows[i]));
        b->setFixedSize(40, 32);
        b->setEnabled(false);
        if (i != 4) {   // центральная кнопка декоративная, движения не делает
            connect(b, &QToolButton::pressed, this, [this, i] {
                const CameraEntry *e = m_mgr->find(m_cameraId);
                if (!e || !e->hasPtz())
                    return;
                const double speed = m_speed->value() / 8.0;
                ptzContinuousMove(m_nam, {e->ptzXAddr, e->ptzToken, e->ptzUser, e->ptzPassword},
                                  kDirX[i] * speed, -kDirY[i] * speed, 0.0);   // y камеры обычно вверх — положительный
            });
            connect(b, &QToolButton::released, this, [this] {
                if (const CameraEntry *e = m_mgr->find(m_cameraId))
                    ptzStop(m_nam, {e->ptzXAddr, e->ptzToken, e->ptzUser, e->ptzPassword});
            });
        } else {
            b->setEnabled(false);
        }
        m_dirs[i] = b;
        lay->addWidget(b, i / 3, i % 3, Qt::AlignCenter);
    }

    m_zoomOut = new QToolButton(box);
    m_zoomOut->setText(QStringLiteral("−"));
    m_zoomOut->setEnabled(false);
    m_zoomIn = new QToolButton(box);
    m_zoomIn->setText(QStringLiteral("+"));
    m_zoomIn->setEnabled(false);
    lay->addWidget(new QLabel(T("Зум"), box), 3, 0);
    lay->addWidget(m_zoomOut, 3, 1, Qt::AlignCenter);
    lay->addWidget(m_zoomIn, 3, 2, Qt::AlignCenter);

    connect(m_zoomIn, &QToolButton::pressed, this, [this] {
        if (const CameraEntry *e = m_mgr->find(m_cameraId))
            ptzContinuousMove(m_nam, {e->ptzXAddr, e->ptzToken, e->ptzUser, e->ptzPassword}, 0, 0, m_speed->value() / 8.0);
    });
    connect(m_zoomOut, &QToolButton::pressed, this, [this] {
        if (const CameraEntry *e = m_mgr->find(m_cameraId))
            ptzContinuousMove(m_nam, {e->ptzXAddr, e->ptzToken, e->ptzUser, e->ptzPassword}, 0, 0, -m_speed->value() / 8.0);
    });
    connect(m_zoomIn, &QToolButton::released, this, [this] {
        if (const CameraEntry *e = m_mgr->find(m_cameraId))
            ptzStop(m_nam, {e->ptzXAddr, e->ptzToken, e->ptzUser, e->ptzPassword});
    });
    connect(m_zoomOut, &QToolButton::released, this, [this] {
        if (const CameraEntry *e = m_mgr->find(m_cameraId))
            ptzStop(m_nam, {e->ptzXAddr, e->ptzToken, e->ptzUser, e->ptzPassword});
    });

    m_speed = new QSlider(Qt::Horizontal, box);
    m_speed->setRange(1, 8);
    m_speed->setValue(4);
    lay->addWidget(new QLabel(T("Скорость"), box), 4, 0);
    lay->addWidget(m_speed, 4, 1, 1, 2);

    m_note = new QLabel(T("Выберите поворотную камеру, добавленную по ONVIF"), box);
    static_cast<QLabel *>(m_note)->setWordWrap(true);
    m_note->setStyleSheet(QStringLiteral("color:#6b7480; font-size:11px;"));
    lay->addWidget(m_note, 5, 0, 1, 3);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(box);

    connect(m_mgr, &CameraManager::camerasChanged, this, [this] { updateEnabled(); });
}

void PtzPanel::setCamera(const QString &cameraId)
{
    m_cameraId = cameraId;
    updateEnabled();
}

void PtzPanel::updateEnabled()
{
    const CameraEntry *e = m_mgr->find(m_cameraId);
    const bool on = e && e->hasPtz();
    for (int i = 0; i < 9; ++i)
        if (i != 4)
            m_dirs[i]->setEnabled(on);
    m_zoomIn->setEnabled(on);
    m_zoomOut->setEnabled(on);
    m_speed->setEnabled(on);
    m_note->setVisible(!on);
}

// ======================================================================
// app/rtsp_quick_dialog.cpp
// ======================================================================
RtspQuickDialog::RtspQuickDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(T("Быстрое добавление по RTSP"));
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setMinimumWidth(520);

    auto *hint = new QLabel(T("Для камер и регистраторов, адрес потока которых у вас уже есть."), this);
    hint->setWordWrap(true);
    hint->setStyleSheet(QStringLiteral("color:#8a8a8e;"));

    m_name = new QLineEdit(this);
    m_url = new QLineEdit(this);
    m_url->setPlaceholderText(QStringLiteral("rtsp://логин:пароль@192.168.1.64:554/..."));
    m_group = new QLineEdit(this);
    m_group->setPlaceholderText(T("необязательно"));
    m_record = new QCheckBox(T("Вести непрерывную запись в архив"), this);
    m_record->setChecked(true);

    auto *form = new QFormLayout;
    form->addRow(T("Название"), m_name);
    form->addRow(T("Адрес RTSP"), m_url);
    form->addRow(T("Группа"), m_group);
    form->addRow(QString(), m_record);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    localizeButtons(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *lay = new QVBoxLayout(this);
    lay->addWidget(hint);
    lay->addLayout(form);
    lay->addWidget(buttons);
}

CameraEntry RtspQuickDialog::entry() const
{
    CameraEntry e;
    e.name = m_name->text().trimmed();
    e.url = m_url->text().trimmed();
    e.group = m_group->text().trimmed();
    e.record = m_record->isChecked();
    return e;
}

void RtspQuickDialog::accept()
{
    const QUrl u(m_url->text().trimmed());
    if (m_name->text().trimmed().isEmpty() || !u.isValid() || u.scheme().compare(QLatin1String("rtsp"), Qt::CaseInsensitive) != 0) {
        QMessageBox::warning(this, windowTitle(), T("Укажите название и корректный адрес, начинающийся с rtsp://"));
        return;
    }
    QDialog::accept();
}

// ======================================================================
// app/settings.cpp
// ======================================================================
namespace {

Cfg::View g_view;
bool g_loaded = false;

QString documentsDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
}

} // namespace

CfgNotifier *Cfg::notifier()
{
    static CfgNotifier n;
    return &n;
}

void Cfg::reload()
{
    QSettings s;
    View v;
    v.lowPower = s.value(QStringLiteral("perf/lowPower"), false).toBool();
    v.maxFps = qBound(1, s.value(QStringLiteral("perf/maxFps"), 15).toInt(), 60);
    v.smooth = s.value(QStringLiteral("perf/smooth"), true).toBool();
    v.timestamp = s.value(QStringLiteral("view/timestamp"), true).toBool();
    v.showInfo = s.value(QStringLiteral("view/showInfo"), true).toBool();
    const QString fmt = s.value(QStringLiteral("view/timeFormat")).toString();
    if (!fmt.isEmpty())
        v.timeFormat = fmt;
    if (v.lowPower) {            // слабый компьютер: меньше кадров, без сглаживания и анимаций
        v.maxFps = qMin(v.maxFps, 8);
        v.smooth = false;
    }
    v.animations = !v.lowPower && s.value(QStringLiteral("ui/animations"), true).toBool();
    g_view = v;
    g_loaded = true;
    Q_EMIT notifier()->changed();
}

const Cfg::View &Cfg::view()
{
    if (!g_loaded)
        reload();
    return g_view;
}

bool Cfg::autostartCameras()
{
    return QSettings().value(QStringLiteral("general/autostart"), true).toBool();
}

QString Cfg::videoDir()
{
    return QSettings().value(QStringLiteral("storage/video"),
                             documentsDir() + QStringLiteral("/PSVMS Archive")).toString();
}

QString Cfg::screenshotDir()
{
    return QSettings().value(QStringLiteral("storage/screenshots"),
                             documentsDir() + QStringLiteral("/PSVMS Screenshots")).toString();
}

// ======================================================================
// app/settings_page.cpp
// ======================================================================
namespace {

struct SettingsWinSize {
    int w;
    int h;
};
const SettingsWinSize kSettingsWinSizes[] = {{0, 0},       {1280, 720},  {1600, 900},  {1920, 1080},
                                             {2560, 1080}, {2560, 1440}, {3440, 1440}, {3840, 2160}};

QCheckBox *settingsCheck(QWidget *parent, const QString &text, const QString &key, bool def)
{
    auto *c = new QCheckBox(text, parent);
    c->setChecked(QSettings().value(key, def).toBool());
    QObject::connect(c, &QCheckBox::toggled, c, [key](bool on) {
        QSettings().setValue(key, on);
        Cfg::reload();
    });
    return c;
}

// строка выбора папки: поле + «Обзор…» + «Открыть»
QWidget *settingsFolderRow(QWidget *parent, QLineEdit *edit, const std::function<void(const QString &)> &onPicked)
{
    auto *w = new QWidget(parent);
    auto *l = new QHBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 0);
    edit->setReadOnly(true);
    auto *browse = new QPushButton(T("Обзор…"), w);
    auto *open = new QPushButton(T("Открыть"), w);
    l->addWidget(edit, 1);
    l->addWidget(browse);
    l->addWidget(open);
    QObject::connect(browse, &QPushButton::clicked, w, [w, edit, onPicked] {
        const QString dir = QFileDialog::getExistingDirectory(w, T("Выберите папку"), edit->text());
        if (!dir.isEmpty()) {
            edit->setText(dir);
            onPicked(dir);
        }
    });
    QObject::connect(open, &QPushButton::clicked, w, [edit] {
        QDesktopServices::openUrl(QUrl::fromLocalFile(edit->text()));
    });
    return w;
}

QWidget *settingsPane(QLayout *layout)
{
    auto *w = new QWidget;
    layout->setContentsMargins(20, 18, 20, 18);
    w->setLayout(layout);
    return w;
}

// Добавление пользователя
class SettingsUserDialog : public QDialog {
public:
    explicit SettingsUserDialog(QWidget *parent) : QDialog(parent)
    {
        setWindowTitle(T("Новый пользователь"));
        setWindowFlag(Qt::WindowContextHelpButtonHint, false);
        setMinimumWidth(420);
        m_name = new QLineEdit(this);
        m_role = new QComboBox(this);
        m_role->addItem(T("Администратор (все разделы)"), QStringLiteral("admin"));
        m_role->addItem(T("Оператор (только просмотр)"), QStringLiteral("operator"));
        m_role->setCurrentIndex(1);
        m_msg = new QLabel(this);
        m_msg->setStyleSheet(QStringLiteral("color:#e8833a;"));
        m_msg->setWordWrap(true);
        auto *hint = new QLabel(T("Новый пользователь будет создан без пароля. При необходимости пароль можно задать позже."), this);
        hint->setWordWrap(true);
        hint->setStyleSheet(QStringLiteral("color:#8a8a8e;"));
        auto *form = new QFormLayout;
        form->addRow(T("Имя"), m_name);
        form->addRow(T("Роль"), m_role);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        localizeButtons(buttons);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        auto *lay = new QVBoxLayout(this);
        lay->addWidget(hint);
        lay->addLayout(form);
        lay->addWidget(m_msg);
        lay->addWidget(buttons);
    }
    void accept() override
    {
        QString err;
        if (Users::add(m_name->text(), m_role->currentData().toString(), QString(), &err).isEmpty()) {
            m_msg->setText(err);
            return;
        }
        QDialog::accept();
    }
private:
    QLineEdit *m_name = nullptr;
    QComboBox *m_role = nullptr;
    QLabel *m_msg = nullptr;
};

// Новый пароль выбранного пользователя (пустой — без пароля)
class SettingsPasswordDialog : public QDialog {
public:
    SettingsPasswordDialog(const QString &userId, const QString &userName, QWidget *parent)
        : QDialog(parent), m_id(userId)
    {
        setWindowTitle(T("Пароль пользователя"));
        setWindowFlag(Qt::WindowContextHelpButtonHint, false);
        setMinimumWidth(400);
        m_pass = new QLineEdit(this);
        m_pass->setEchoMode(QLineEdit::Password);
        m_pass2 = new QLineEdit(this);
        m_pass2->setEchoMode(QLineEdit::Password);
        m_msg = new QLabel(this);
        m_msg->setStyleSheet(QStringLiteral("color:#e8833a;"));
        m_msg->setWordWrap(true);

        auto *hint = new QLabel(T("Новый пароль для пользователя «%1». Оставьте пустым, чтобы убрать пароль.").arg(userName), this);
        hint->setWordWrap(true);
        auto *form = new QFormLayout;
        form->addRow(T("Новый пароль"), m_pass);
        form->addRow(T("Повторите пароль"), m_pass2);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        localizeButtons(buttons);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        auto *lay = new QVBoxLayout(this);
        lay->addWidget(hint);
        lay->addLayout(form);
        lay->addWidget(m_msg);
        lay->addWidget(buttons);
    }

    void accept() override
    {
        if (m_pass->text() != m_pass2->text()) {
            m_msg->setText(T("Пароли не совпадают."));
            return;
        }
        QString err;
        if (!Users::setPassword(m_id, m_pass->text(), &err)) {
            m_msg->setText(err);
            return;
        }
        QDialog::accept();
    }

private:
    QString m_id;
    QLineEdit *m_pass = nullptr;
    QLineEdit *m_pass2 = nullptr;
    QLabel *m_msg = nullptr;
};

} // namespace

// Диалог встроенного ИИ-помощника PSVMS на базе Yandex AI Studio.
class YandexAssistantDialog final : public QDialog {
public:
    YandexAssistantDialog(const QString &apiKey, const QString &folderId, QWidget *parent = nullptr)
        : QDialog(parent), m_apiKey(apiKey), m_folderId(folderId), m_nam(new QNetworkAccessManager(this))
    {
        setWindowTitle(T("ИИ-помощник PSVMS — Yandex"));
        setMinimumSize(680, 500);
        m_chat = new QTextEdit(this);
        m_chat->setReadOnly(true);
        m_chat->setPlaceholderText(T("Здесь появятся ответы ИИ-помощника."));
        m_input = new QLineEdit(this);
        m_input->setPlaceholderText(T("Напишите вопрос о камерах, регистраторах или настройке PSVMS…"));
        auto *send = new QPushButton(T("Отправить"), this);
        auto *hint = new QLabel(T("Ответы генерируются Yandex AI Studio. Не отправляйте пароли и секретные данные."), this);
        hint->setWordWrap(true);
        hint->setStyleSheet(QStringLiteral("color:#92929a;"));
        auto *row = new QHBoxLayout;
        row->addWidget(m_input, 1);
        row->addWidget(send);
        auto *lay = new QVBoxLayout(this);
        lay->addWidget(m_chat, 1);
        lay->addLayout(row);
        lay->addWidget(hint);
        connect(send, &QPushButton::clicked, this, [this] { sendMessage(); });
        connect(m_input, &QLineEdit::returnPressed, this, [this] { sendMessage(); });
        m_history.append(QJsonObject{{QStringLiteral("role"), QStringLiteral("system")},
            {QStringLiteral("text"), QStringLiteral("Ты — технический помощник программы PSVMS для видеонаблюдения. Помогай диагностировать IP-камеры и видеорегистраторы, RTSP, ONVIF, TCP/UDP порты, сеть и настройки PSVMS. Давай пошаговые и безопасные инструкции. Не выдумывай точные порты конкретной модели, если модель неизвестна; проси уточнение. Предупреждай, что открытие портов в интернет повышает риск и предпочтительнее VPN. Отвечай по-русски, если пользователь пишет по-русски.")}});
        m_chat->append(QStringLiteral("<b>PSVMS AI</b><br>%1").arg(T("Здравствуйте! Помогу с подключением камер, RTSP/ONVIF, портами и настройкой программы.")));
    }
private:
    void sendMessage()
    {
        const QString prompt = m_input->text().trimmed();
        if (prompt.isEmpty() || m_busy) return;
        m_input->clear();
        m_chat->append(QStringLiteral("<p style='color:#d8bd82'><b>%1</b><br>%2</p>")
                       .arg(T("Вы"), prompt.toHtmlEscaped()));
        m_history.append(QJsonObject{{QStringLiteral("role"), QStringLiteral("user")}, {QStringLiteral("text"), prompt}});
        QJsonArray messages;
        for (const auto &v : m_history) messages.append(v);
        QJsonObject body;
        body.insert(QStringLiteral("modelUri"), QStringLiteral("gpt://%1/yandexgpt-5.1").arg(m_folderId));
        body.insert(QStringLiteral("completionOptions"), QJsonObject{{QStringLiteral("stream"), false},
            {QStringLiteral("temperature"), 0.25}, {QStringLiteral("maxTokens"), QStringLiteral("1800")}});
        body.insert(QStringLiteral("messages"), messages);
        QNetworkRequest req(QUrl(QStringLiteral("https://ai.api.cloud.yandex.net/foundationModels/v1/completion")));
        req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        req.setRawHeader("Authorization", QByteArray("Api-Key ") + m_apiKey.toUtf8());
        req.setRawHeader("x-folder-id", m_folderId.toUtf8());
        m_busy = true;
        m_input->setEnabled(false);
        auto *reply = m_nam->post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
        connect(reply, &QNetworkReply::finished, this, [this, reply] {
            m_busy = false;
            m_input->setEnabled(true);
            const QByteArray raw = reply->readAll();
            if (reply->error() != QNetworkReply::NoError) {
                m_chat->append(QStringLiteral("<p style='color:#ef7777'><b>%1</b><br>%2</p>")
                    .arg(T("Ошибка Yandex AI"), QString::fromUtf8(raw).toHtmlEscaped().left(1200)));
                reply->deleteLater();
                return;
            }
            const QJsonObject root = QJsonDocument::fromJson(raw).object();
            const QJsonArray alternatives = root.value(QStringLiteral("alternatives")).toArray();
            QString answer;
            if (!alternatives.isEmpty())
                answer = alternatives.first().toObject().value(QStringLiteral("message")).toObject()
                    .value(QStringLiteral("text")).toString();
            if (answer.isEmpty()) answer = T("Сервис вернул пустой ответ. Проверьте модель, каталог и права API-ключа.");
            m_chat->append(QStringLiteral("<p><b style='color:#8ec6a0'>Yandex AI</b><br>%1</p>")
                           .arg(answer.toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br>"))));
            m_history.append(QJsonObject{{QStringLiteral("role"), QStringLiteral("assistant")}, {QStringLiteral("text"), answer}});
            // Ограничиваем историю, чтобы запросы оставались компактными.
            while (m_history.size() > 17) m_history.removeAt(1);
            reply->deleteLater();
        });
    }
    QString m_apiKey, m_folderId;
    QNetworkAccessManager *m_nam = nullptr;
    QTextEdit *m_chat = nullptr;
    QLineEdit *m_input = nullptr;
    QJsonArray m_history;
    bool m_busy = false;
};

SettingsPage::SettingsPage(CameraManager *mgr, QWidget *parent) : QWidget(parent), m_mgr(mgr)
{
    auto *tabs = new QTabWidget(this);
    tabs->addTab(buildGeneralTab(), T("Общие"));
    tabs->addTab(buildVideoTab(), T("Видео и папки"));
    tabs->addTab(buildPerformanceTab(), T("Производительность"));
    tabs->addTab(buildUsersTab(), T("Пользователи"));
    tabs->addTab(buildBackupTab(), T("Резервная копия"));
    tabs->addTab(buildLicenseTab(), T("Лицензия"));
    tabs->addTab(buildAboutTab(), T("О программе"));
    tabs->addTab(buildNetworkTab(), T("Сеть и порты"));
    tabs->addTab(buildAiTab(), T("ИИ-помощник"));
    for (int i = 1; i <= 4; ++i)
        tabs->setTabEnabled(i, isProVersion());

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(16, 12, 16, 12);
    lay->addWidget(tabs);
}

// ---------------------------------------------------------------- общие

QWidget *SettingsPage::buildGeneralTab()
{
    auto *form = new QFormLayout;
    form->setLabelAlignment(Qt::AlignLeft);
    QSettings s;

    auto *lang = new QComboBox;
    lang->addItem(QString::fromUtf8("Русский"), QStringLiteral("ru"));
    lang->addItem(QStringLiteral("English"), QStringLiteral("en"));
    lang->setCurrentIndex(lang->findData(I18n::code()));
    connect(lang, &QComboBox::activated, this, [this, lang](int i) {
        const QString code = lang->itemData(i).toString();
        if (code != I18n::code())
            emit languageChosen(code);
    });
    form->addRow(T("Язык интерфейса"), lang);

    form->addRow(QString(), settingsCheck(this, T("Запускать камеры автоматически при запуске программы"),
                                          QStringLiteral("general/autostart"), true));
    form->addRow(QString(), settingsCheck(this, T("Открывать программу на весь экран"),
                                          QStringLiteral("ui/startFullscreen"), false));
    form->addRow(QString(), settingsCheck(this, T("Показывать счётчики «В сети» и «Не в сети» и кнопку «Состояние камер»"),
                                          QStringLiteral("ui/showCameraStatus"), true));

    // размер окна, в том числе для широкоформатных (21:9) мониторов
    auto *size = new QComboBox;
    size->addItem(T("Авто"), 0);
    const char *labels[] = {"", "1280 × 720", "1600 × 900", "1920 × 1080 (16:9)", "2560 × 1080 (21:9)",
                            "2560 × 1440 (16:9)", "3440 × 1440 (21:9)", "3840 × 2160 (4K)"};
    for (int i = 1; i < int(sizeof(kSettingsWinSizes) / sizeof(kSettingsWinSizes[0])); ++i)
        size->addItem(QString::fromUtf8(labels[i]), i);
    size->setCurrentIndex(qBound(0, s.value(QStringLiteral("ui/windowSize"), 0).toInt(), size->count() - 1));
    connect(size, &QComboBox::activated, this, [this, size](int i) {
        QSettings().setValue(QStringLiteral("ui/windowSize"), i);
        QWidget *w = window();
        if (i > 0 && !w->isFullScreen() && !w->isMaximized())
            w->resize(kSettingsWinSizes[i].w, kSettingsWinSizes[i].h);
    });
    form->addRow(T("Размер окна"), size);

    auto *scale = new QComboBox;
    scale->addItem(T("Авто"), 0.0);
    for (double v : {1.0, 1.1, 1.25, 1.5, 1.75, 2.0})
        scale->addItem(QStringLiteral("%1%").arg(int(v * 100)), v);
    const int sc = scale->findData(s.value(QStringLiteral("ui/scale"), 0.0).toDouble());
    scale->setCurrentIndex(sc >= 0 ? sc : 0);
    // Значения масштабирования редактируются в форме и записываются только
    // после нажатия кнопки «Применить изменения».
    form->addRow(T("Масштаб интерфейса (пресет, после перезапуска)"), scale);

    // Ручная настройка размеров интерфейса, текста и интерактивных элементов.
    auto makePercentSpin = [this](const QString &key, int fallback) {
        auto *spin = new QSpinBox(this);
        spin->setRange(75, 200);
        spin->setSingleStep(5);
        spin->setSuffix(QStringLiteral("%"));
        spin->setValue(qBound(75, QSettings().value(key, fallback).toInt(), 200));
        Q_UNUSED(key);
        return spin;
    };
    auto *manualInterface = makePercentSpin(QStringLiteral("ui/manualInterfacePercent"), 100);
    auto *manualText = makePercentSpin(QStringLiteral("ui/manualTextPercent"), 100);
    auto *manualElements = makePercentSpin(QStringLiteral("ui/manualElementsPercent"), 100);
    form->addRow(T("Ручной размер интерфейса (после перезапуска)"), manualInterface);
    form->addRow(T("Размер текста (после перезапуска)"), manualText);
    form->addRow(T("Размер кнопок и элементов (после перезапуска)"), manualElements);

    auto *resetScaling = new QPushButton(T("Вернуть масштабирование к стандартным настройкам"), this);
    resetScaling->setToolTip(T("Сбросить пресет, ручной масштаб, размер текста и элементов до 100%"));
    connect(resetScaling, &QPushButton::clicked, this,
            [scale, manualInterface, manualText, manualElements]() {
        // Сброс пока меняет только значения в форме. Для записи нажмите «Применить изменения».
        scale->setCurrentIndex(0);
        manualInterface->setValue(100);
        manualText->setValue(100);
        manualElements->setValue(100);
    });
    form->addRow(QString(), resetScaling);

    auto *applyScaling = new QPushButton(T("Применить изменения"), this);
    applyScaling->setToolTip(T("Сохранить выбранные параметры масштабирования"));
    connect(applyScaling, &QPushButton::clicked, this,
            [scale, manualInterface, manualText, manualElements]() {
        QSettings settings;
        settings.setValue(QStringLiteral("ui/scale"), scale->currentData().toDouble());
        settings.setValue(QStringLiteral("ui/manualInterfacePercent"), manualInterface->value());
        settings.setValue(QStringLiteral("ui/manualTextPercent"), manualText->value());
        settings.setValue(QStringLiteral("ui/manualElementsPercent"), manualElements->value());
        settings.sync();
        QMessageBox::information(nullptr, T("Масштабирование"),
                                 T("Изменения сохранены. Новое масштабирование применится после перезапуска программы."));
    });
    form->addRow(QString(), applyScaling);

    form->addRow(QString(), settingsCheck(this, T("Показывать дату и время на камерах"), QStringLiteral("view/timestamp"), true));
    auto *fmt = new QComboBox;
    const QStringList formats = {QStringLiteral("dd.MM.yyyy HH:mm:ss"), QStringLiteral("yyyy-MM-dd HH:mm:ss"),
                                 QStringLiteral("HH:mm:ss  dd.MM.yyyy"), QStringLiteral("dd/MM/yyyy hh:mm:ss AP")};
    fmt->addItems(formats);
    const int fi = formats.indexOf(s.value(QStringLiteral("view/timeFormat"), formats.first()).toString());
    fmt->setCurrentIndex(fi >= 0 ? fi : 0);
    connect(fmt, &QComboBox::activated, this, [fmt](int i) {
        QSettings().setValue(QStringLiteral("view/timeFormat"), fmt->itemText(i));
        Cfg::reload();
    });
    form->addRow(T("Формат даты и времени"), fmt);

    auto *hotkeys = new QLabel(
        T("Горячие клавиши: F11 — во весь экран, F5 — обновить окна с камерами, "
          "1 / 2 / 3 / 4 — раскладка 1, 2×2, 3×3, 4×4, Delete — закрыть выбранную камеру "
          "(в разделе «Мониторинг»)."), this);
    hotkeys->setWordWrap(true);
    hotkeys->setStyleSheet(QStringLiteral("color:#8a8a8e; margin-top: 10px;"));

    auto *lay = new QVBoxLayout;
    lay->addLayout(form);
    lay->addWidget(hotkeys);
    lay->addStretch(1);
    return settingsPane(lay);
}

// ---------------------------------------------------------------- видео и папки

QWidget *SettingsPage::buildVideoTab()
{
    auto *form = new QFormLayout;

    auto *videoEdit = new QLineEdit(m_mgr->storageRoot());
    form->addRow(T("Папка для записей видео"), settingsFolderRow(this, videoEdit, [this](const QString &dir) {
        m_mgr->setStorageRoot(dir);         // запущенные рекордеры перезапускаются в новую папку
        emit storageChanged();
    }));

    auto *shotEdit = new QLineEdit(Cfg::screenshotDir());
    form->addRow(T("Папка для скриншотов"), settingsFolderRow(this, shotEdit, [](const QString &dir) {
        QSettings().setValue(QStringLiteral("storage/screenshots"), dir);
    }));

    form->addRow(QString(), settingsCheck(this, T("Показывать название, FPS, разрешение и качество связи поверх камеры"),
                                          QStringLiteral("view/showInfo"), true));

    auto *note = new QLabel(T("Записи ведутся сегментами по 5 минут. При смене папки новые записи пишутся в неё, "
                              "старые остаются на прежнем месте. Скриншот делается из меню камеры (правая кнопка мыши)."), this);
    note->setWordWrap(true);
    note->setStyleSheet(QStringLiteral("color:#8a8a8e;"));

    auto *lay = new QVBoxLayout;
    lay->addLayout(form);
    lay->addWidget(note);
    lay->addStretch(1);
    return settingsPane(lay);
}

// ---------------------------------------------------------------- производительность

QWidget *SettingsPage::buildPerformanceTab()
{
    auto *form = new QFormLayout;
    form->addRow(QString(), settingsCheck(this, T("Режим для слабого компьютера"), QStringLiteral("perf/lowPower"), false));

    auto *fps = new QSpinBox;
    fps->setRange(1, 30);
    fps->setValue(QSettings().value(QStringLiteral("perf/maxFps"), 15).toInt());
    connect(fps, &QSpinBox::valueChanged, this, [](int v) {
        QSettings().setValue(QStringLiteral("perf/maxFps"), v);
        Cfg::reload();
    });
    form->addRow(T("Предел кадров в секунду на ячейку"), fps);

    form->addRow(QString(), settingsCheck(this, T("Сглаживание картинки"), QStringLiteral("perf/smooth"), true));
    form->addRow(QString(), settingsCheck(this, T("Анимация интерфейса (заставка, Demo, значки и переходы)"),
                                          QStringLiteral("ui/animations"), true));

    auto *note = new QLabel(T("Режим для слабого компьютера: не более 8 кадров в секунду, без сглаживания и анимаций, "
                              "реже проверяется связь с камерами. Ещё сильнее нагрузку снижает показ подпотока "
                              "(у камеры: «Качество картинки → Низкое») и меньшее число ячеек в раскладке."), this);
    note->setWordWrap(true);
    note->setStyleSheet(QStringLiteral("color:#8a8a8e;"));

    auto *lay = new QVBoxLayout;
    lay->addLayout(form);
    lay->addWidget(note);
    lay->addStretch(1);
    return settingsPane(lay);
}

// ---------------------------------------------------------------- диагностика сети и портов

QWidget *SettingsPage::buildNetworkTab()
{
    auto *host = new QLineEdit(this);
    host->setPlaceholderText(QStringLiteral("192.168.1.64"));
    auto *port = new QSpinBox(this);
    port->setRange(1, 65535);
    port->setValue(554);
    auto *protocol = new QComboBox(this);
    protocol->addItem(QStringLiteral("TCP"), QStringLiteral("tcp"));
    protocol->addItem(QStringLiteral("UDP (проверка отправки)"), QStringLiteral("udp"));
    auto *vendor = new QComboBox(this);
    vendor->addItems({QStringLiteral("Универсальные / ONVIF"), QStringLiteral("Hikvision / HiWatch"),
                      QStringLiteral("Dahua"), QStringLiteral("Uniview"), QStringLiteral("Другой производитель")});
    auto *model = new QLineEdit(this);
    model->setPlaceholderText(T("Модель камеры или регистратора (необязательно)"));
    auto *service = new QComboBox(this);
    service->addItem(T("RTSP — видеопоток"), 554);
    service->addItem(T("HTTP — веб-интерфейс"), 80);
    service->addItem(T("HTTPS — защищённый веб-интерфейс"), 443);
    service->addItem(T("ONVIF — обнаружение/управление (часто HTTP)"), 80);
    service->addItem(T("Hikvision — SDK / Server"), 8000);
    service->addItem(T("Dahua — TCP Service"), 37777);
    service->addItem(T("Пользовательский порт"), 0);
    auto *result = new QLabel(T("Укажите IP-адрес и порт, затем нажмите «Проверить порт»."), this);
    result->setWordWrap(true);
    result->setMinimumHeight(54);
    result->setStyleSheet(QStringLiteral("QLabel{padding:12px;border-radius:10px;background:#25272d;color:#d8bd82;}"));
    auto *advice = new QLabel(this);
    advice->setWordWrap(true);
    advice->setText(T("Подсказки по типовым портам:\n• RTSP: 554/TCP — видеопоток большинства IP-камер.\n• HTTP: 80/TCP; HTTPS: 443/TCP — веб-интерфейс.\n• Hikvision/HiWatch: 8000/TCP — SDK/Server, 554/TCP — RTSP.\n• Dahua: 37777/TCP — сервисный порт, 554/TCP — RTSP.\n• ONVIF часто использует HTTP-порт устройства (обычно 80 или заданный вручную).\nПорты могут быть изменены производителем, установщиком или администратором. Сверяйтесь с сетевыми настройками конкретной модели."));
    advice->setStyleSheet(QStringLiteral("color:#a7a7ad;"));
    auto *test = new QPushButton(T("Проверить порт"), this);
    auto *fill = new QPushButton(T("Подставить порт службы"), this);
    auto *form = new QFormLayout;
    form->addRow(T("IP-адрес / имя узла"), host);
    form->addRow(T("Производитель"), vendor);
    form->addRow(T("Модель"), model);
    form->addRow(T("Тип службы"), service);
    form->addRow(T("Номер порта"), port);
    form->addRow(T("Протокол"), protocol);
    auto *buttons = new QHBoxLayout;
    buttons->addWidget(fill);
    buttons->addWidget(test);
    auto *lay = new QVBoxLayout;
    lay->addLayout(form);
    lay->addLayout(buttons);
    lay->addWidget(result);
    lay->addWidget(advice);
    lay->addStretch(1);
    connect(service, qOverload<int>(&QComboBox::currentIndexChanged), this, [service, port](int) {
        const int p = service->currentData().toInt();
        if (p > 0) port->setValue(p);
    });
    connect(fill, &QPushButton::clicked, this, [service, port] {
        const int p = service->currentData().toInt();
        if (p > 0) port->setValue(p);
    });
    connect(vendor, qOverload<int>(&QComboBox::currentIndexChanged), this, [vendor, advice] {
        const QString v = vendor->currentText();
        if (v.contains(QStringLiteral("Hikvision")))
            advice->setText(T("Hikvision / HiWatch: обычно HTTP 80/TCP, HTTPS 443/TCP, RTSP 554/TCP, SDK/Server 8000/TCP. В некоторых моделях есть дополнительные порты 8443, 7681 и 7682. Проверьте меню Configuration → Network → Basic Settings → Port и документацию именно вашей модели."));
        else if (v.contains(QStringLiteral("Dahua")))
            advice->setText(T("Dahua: часто HTTP 80/TCP, RTSP 554/TCP, TCP Service 37777/TCP. Порты зависят от модели и прошивки; уточните их в Network → Connection → Port или в руководстве устройства."));
        else if (v.contains(QStringLiteral("Uniview")))
            advice->setText(T("Uniview: обычно используются HTTP/HTTPS для веб-доступа и RTSP 554/TCP для потока; порт управления зависит от модели и версии прошивки. Проверьте Network → Port в веб-интерфейсе."));
        else
            advice->setText(T("Типовые значения: RTSP 554/TCP, HTTP 80/TCP, HTTPS 443/TCP, ONVIF — часто HTTP-порт устройства. Порты управления зависят от производителя и модели; не применяйте значения Hikvision/Dahua к другому устройству без проверки документации."));
    });
    connect(test, &QPushButton::clicked, this, [this, host, port, protocol, result, test] {
        const QString address = host->text().trimmed();
        const int number = port->value();
        if (address.isEmpty()) { result->setText(T("Введите IP-адрес или имя узла.")); return; }
        test->setEnabled(false);
        if (protocol->currentData().toString() == QStringLiteral("udp")) {
            auto *sock = new QUdpSocket(this);
            const QHostAddress ip(address);
            if (!ip.isNull()) {
                const QByteArray probe("PSVMS-port-check");
                const qint64 sent = sock->writeDatagram(probe, ip, quint16(number));
                result->setText(sent == probe.size()
                    ? T("UDP-пакет отправлен. Это НЕ подтверждает, что порт открыт: UDP не устанавливает соединение, а ответ зависит от службы устройства.")
                    : T("Не удалось отправить UDP-пакет. Проверьте IP, маршрут и настройки сети."));
            } else {
                result->setText(T("Для UDP-проверки укажите IP-адрес в числовом формате."));
            }
            sock->deleteLater();
            test->setEnabled(true);
            return;
        }
        auto *socket = new QTcpSocket(this);
        auto done = std::make_shared<bool>(false);
        auto finish = [socket, test, result, address, number, done](bool open, const QString &detail) {
            if (*done) return;
            *done = true;
            result->setText(QStringLiteral("%1:%2 — %3\n%4").arg(address).arg(number)
                .arg(open ? T("ПОРТ ДОСТУПЕН") : T("ПОРТ НЕДОСТУПЕН"), detail));
            result->setStyleSheet(open
                ? QStringLiteral("QLabel{padding:12px;border-radius:10px;background:#203b31;color:#9fe0b6;}")
                : QStringLiteral("QLabel{padding:12px;border-radius:10px;background:#43282a;color:#ffaaaa;}") );
            test->setEnabled(true);
            socket->deleteLater();
        };
        connect(socket, &QTcpSocket::connected, this, [finish] { finish(true, T("TCP-соединение установлено. Служба принимает подключения на этом порту.")); });
        connect(socket, &QTcpSocket::errorOccurred, this, [finish, socket](QAbstractSocket::SocketError) {
            finish(false, socket->errorString() + T(". Возможные причины: служба выключена, порт изменён, устройство недоступно или соединение блокирует firewall/маршрутизатор."));
        });
        QTimer::singleShot(4500, socket, [finish, socket] {
            if (socket->state() != QAbstractSocket::ConnectedState)
                finish(false, T("Истекло время ожидания. Устройство не ответило на TCP-подключение за 4,5 секунды."));
        });
        socket->connectToHost(address, quint16(number));
    });
    return settingsPane(lay);
}

// ---------------------------------------------------------------- ИИ-помощник Yandex

QWidget *SettingsPage::buildAiTab()
{
    QSettings settings;
    auto *enabled = new QCheckBox(T("Включить ИИ-помощника Yandex в программе"), this);
    enabled->setChecked(settings.value(QStringLiteral("ai/yandexEnabled"), false).toBool());
    auto *key = new QLineEdit(this);
    key->setEchoMode(QLineEdit::Password);
    key->setPlaceholderText(T("Секретный API-ключ Yandex AI Studio"));
    key->setText(SecureStore::decrypt(settings.value(QStringLiteral("ai/yandexApiKey")).toString()));
    auto *folder = new QLineEdit(this);
    folder->setPlaceholderText(T("Идентификатор каталога Yandex Cloud (Folder ID)"));
    folder->setText(settings.value(QStringLiteral("ai/yandexFolderId")).toString());
    auto *save = new QPushButton(T("Сохранить параметры"), this);
    auto *open = new QPushButton(T("Открыть ИИ-чат"), this);
    auto *status = new QLabel(this);
    status->setWordWrap(true);
    status->setStyleSheet(QStringLiteral("color:#a7a7ad;"));
    auto *note = new QLabel(T("Для работы нужен API-ключ сервисного аккаунта Yandex AI Studio с доступом к генерации текста и Folder ID. Используется модель YandexGPT Pro 5.1. Запросы отправляются в облако Yandex; тарификация зависит от вашего аккаунта и условий Yandex Cloud. При выключении переключателя чат не открывается и запросы не отправляются."), this);
    note->setWordWrap(true);
    note->setStyleSheet(QStringLiteral("color:#a7a7ad;"));
    auto *form = new QFormLayout;
    form->addRow(T("API-ключ"), key);
    form->addRow(T("Folder ID"), folder);
    auto *buttons = new QHBoxLayout;
    buttons->addWidget(save);
    buttons->addWidget(open);
    auto *lay = new QVBoxLayout;
    lay->addWidget(enabled);
    lay->addLayout(form);
    lay->addLayout(buttons);
    lay->addWidget(status);
    lay->addWidget(note);
    lay->addStretch(1);
    auto saveSettings = [enabled, key, folder, status] {
        QSettings s;
        s.setValue(QStringLiteral("ai/yandexEnabled"), enabled->isChecked());
        s.setValue(QStringLiteral("ai/yandexApiKey"), SecureStore::encrypt(key->text().trimmed()));
        s.setValue(QStringLiteral("ai/yandexFolderId"), folder->text().trimmed());
        status->setText(T("Параметры сохранены."));
    };
    connect(save, &QPushButton::clicked, this, saveSettings);
    connect(enabled, &QCheckBox::toggled, this, [saveSettings, open](bool on) {
        saveSettings();
        open->setEnabled(on);
    });
    open->setEnabled(enabled->isChecked());
    connect(open, &QPushButton::clicked, this, [this, enabled, key, folder, status, saveSettings] {
        saveSettings();
        if (!enabled->isChecked()) { status->setText(T("ИИ-помощник отключён в настройках.")); return; }
        if (key->text().trimmed().isEmpty() || folder->text().trimmed().isEmpty()) {
            status->setText(T("Сначала укажите API-ключ и Folder ID."));
            return;
        }
        YandexAssistantDialog dlg(key->text().trimmed(), folder->text().trimmed(), this);
        dlg.exec();
    });
    return settingsPane(lay);
}

// ---------------------------------------------------------------- пользователи

QString SettingsPage::currentUserId() const
{
    const int row = m_users->currentRow();
    return row < 0 ? QString() : m_users->item(row, 0)->data(Qt::UserRole).toString();
}

QWidget *SettingsPage::buildUsersTab()
{
    m_users = new QTableWidget(0, 4, this);
    m_users->setHorizontalHeaderLabels({T("Имя"), T("Роль"), T("Пароль"), T("Состояние")});
    m_users->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_users->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_users->setSelectionMode(QAbstractItemView::SingleSelection);
    m_users->verticalHeader()->setVisible(false);
    m_users->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_users->setColumnWidth(1, 180);
    m_users->setColumnWidth(2, 110);
    m_users->setColumnWidth(3, 120);

    auto *add = new QPushButton(T("Добавить пользователя…"), this);
    auto *pass = new QPushButton(T("Пароль…"), this);
    auto *role = new QPushButton(T("Роль…"), this);
    auto *toggle = new QPushButton(T("Отключить / включить"), this);
    auto *audit = new QPushButton(T("Журнал действий…"), this);
    auto *del = new QPushButton(T("Удалить"), this);
    auto *buttons = new QHBoxLayout;
    buttons->addWidget(add);
    buttons->addWidget(pass);
    buttons->addWidget(role);
    buttons->addWidget(toggle);
    buttons->addWidget(audit);
    buttons->addWidget(del);
    buttons->addStretch(1);

    auto *note = new QLabel(T("Гость может только просматривать камеры. Новые пользователи создаются без пароля; Administrator может установить пароль при первом входе. "
                              "Разработчик всегда защищён обязательным паролем. Отключённые пользователи не могут войти. Журнал фиксирует входы и действия."), this);
    note->setWordWrap(true);
    note->setStyleSheet(QStringLiteral("color:#8a8a8e;"));

    auto *idleBox = new QGroupBox(T("Автоматическая блокировка"));
    auto *idleEnabled = new QCheckBox(T("Блокировать PSVMS при бездействии"), idleBox);
    auto *idleMinutes = new QSpinBox(idleBox);
    idleMinutes->setRange(1, 240);
    idleMinutes->setSuffix(T(" мин."));
    idleMinutes->setValue(QSettings().value(QStringLiteral("security/idleLockMinutes"), 10).toInt());
    idleEnabled->setChecked(QSettings().value(QStringLiteral("security/idleLockEnabled"), false).toBool());
    idleMinutes->setEnabled(idleEnabled->isChecked());
    auto *idleRow = new QHBoxLayout(idleBox);
    idleRow->addWidget(idleEnabled);
    idleRow->addStretch(1);
    idleRow->addWidget(new QLabel(T("Время:"), idleBox));
    idleRow->addWidget(idleMinutes);
    connect(idleEnabled, &QCheckBox::toggled, idleMinutes, &QWidget::setEnabled);
    auto saveIdle = [idleEnabled, idleMinutes] {
        QSettings settings;
        settings.setValue(QStringLiteral("security/idleLockEnabled"), idleEnabled->isChecked());
        settings.setValue(QStringLiteral("security/idleLockMinutes"), idleMinutes->value());
        settings.sync();
        QCoreApplication::postEvent(qApp, new QEvent(QEvent::User));
        SecurityAudit::write(QStringLiteral("idle_lock_settings_changed"),
            QStringLiteral("enabled=%1 minutes=%2").arg(idleEnabled->isChecked() ? 1 : 0).arg(idleMinutes->value()));
    };
    connect(idleEnabled, &QCheckBox::toggled, this, [saveIdle](bool) { saveIdle(); });
    connect(idleMinutes, qOverload<int>(&QSpinBox::valueChanged), this, [saveIdle](int) { saveIdle(); });

    auto *lay = new QVBoxLayout;
    lay->addWidget(idleBox);
    lay->addLayout(buttons);
    lay->addWidget(m_users, 1);
    lay->addWidget(note);

    if (!Users::current().isAdmin()) {           // управлять пользователями может только администратор
        add->setEnabled(false);
        pass->setEnabled(false);
        role->setEnabled(false);
        toggle->setEnabled(false);
        audit->setEnabled(false);
        del->setEnabled(false);
    }
    connect(add, &QPushButton::clicked, this, [this] {
        SettingsUserDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted)
            rebuildUsers();
    });
    connect(pass, &QPushButton::clicked, this, [this] {
        const UserInfo u = Users::find(currentUserId());
        if (!u.valid())
            return;
        SettingsPasswordDialog dlg(u.id, u.name, this);
        if (dlg.exec() == QDialog::Accepted)
            rebuildUsers();
    });
    connect(role, &QPushButton::clicked, this, [this] {
        const UserInfo u = Users::find(currentUserId());
        if (!u.valid())
            return;
        QMenu menu(this);
        QAction *asAdmin = menu.addAction(T("Администратор (все разделы)"));
        QAction *asOperator = menu.addAction(T("Оператор (только просмотр)"));
        asAdmin->setCheckable(true);
        asOperator->setCheckable(true);
        asAdmin->setChecked(u.isAdmin());
        asOperator->setChecked(!u.isAdmin());
        QAction *chosen = menu.exec(QCursor::pos());
        if (!chosen)
            return;
        const QString newRole = chosen == asAdmin ? QStringLiteral("admin") : QStringLiteral("operator");
        QString err;
        if (!Users::setRole(u.id, newRole, &err))
            QMessageBox::warning(this, T("Роль пользователя"), err);
        rebuildUsers();
    });
    auto updateToggleText = [this, toggle] {
        const UserInfo u = Users::find(currentUserId());
        toggle->setText(!u.valid() ? T("Включить / отключить")
                                  : u.enabled ? T("Временно отключить") : T("Включить пользователя"));
        toggle->setEnabled(Users::current().isAdmin() && u.valid()
                           && !u.isGuest() && !u.isDeveloper()
                           && u.id != Users::current().id);
    };
    connect(m_users, &QTableWidget::itemSelectionChanged, this, updateToggleText);
    connect(toggle, &QPushButton::clicked, this, [this, updateToggleText] {
        const UserInfo u = Users::find(currentUserId());
        if (!u.valid()) return;
        QString err;
        if (!Users::setEnabled(u.id, !u.enabled, &err))
            QMessageBox::warning(this, T("Состояние пользователя"), err);
        rebuildUsers();
        updateToggleText();
    });
    connect(audit, &QPushButton::clicked, this, [this] {
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        QFile file(dir + QStringLiteral("/security-audit.log"));
        QString content;
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            content = QString::fromUtf8(file.readAll());
        else
            content = T("Журнал пока пуст или недоступен.");
        QDialog dlg(this);
        dlg.setWindowTitle(T("Журнал входов и действий"));
        dlg.resize(900, 560);
        auto *view = new QPlainTextEdit(&dlg);
        view->setReadOnly(true);
        view->setPlainText(content);
        auto *close = new QPushButton(T("Закрыть"), &dlg);
        connect(close, &QPushButton::clicked, &dlg, &QDialog::accept);
        auto *layout = new QVBoxLayout(&dlg);
        layout->addWidget(view);
        layout->addWidget(close, 0, Qt::AlignRight);
        dlg.exec();
    });
    connect(del, &QPushButton::clicked, this, [this] {
        const UserInfo u = Users::find(currentUserId());
        if (!u.valid())
            return;
        if (u.isGuest() || u.isDeveloper()) {
            QMessageBox::information(this, T("Удаление пользователя"),
                u.isGuest() ? T("Системную учётную запись «Гость» нельзя удалить.")
                            : T("Учётную запись «Разработчик» нельзя удалить."));
            return;
        }
        QMessageBox box(QMessageBox::Question, T("Удаление пользователя"),
                        T("Удалить пользователя «%1»?").arg(u.name), QMessageBox::NoButton, this);
        QPushButton *yes = box.addButton(T("Удалить"), QMessageBox::AcceptRole);
        box.addButton(T("Отмена"), QMessageBox::RejectRole);
        box.exec();
        if (box.clickedButton() != yes)
            return;
        QString err;
        if (!Users::remove(u.id, &err))
            QMessageBox::warning(this, T("Удаление пользователя"), err);
        rebuildUsers();
    });

    rebuildUsers();
    updateToggleText();
    return settingsPane(lay);
}

void SettingsPage::rebuildUsers()
{
    m_users->setRowCount(0);
    for (const UserInfo &u : Users::all()) {
        const int row = m_users->rowCount();
        m_users->insertRow(row);
        auto *name = new QTableWidgetItem(u.name);
        name->setData(Qt::UserRole, u.id);
        m_users->setItem(row, 0, name);
        m_users->setItem(row, 1, new QTableWidgetItem(u.isDeveloper() ? T("Разработчик") :
            u.isGuest() ? T("Гость") : u.isAdmin() ? T("Администратор") : T("Оператор")));
        m_users->setItem(row, 2, new QTableWidgetItem(u.hasPassword ? T("задан") : T("без пароля")));
        m_users->setItem(row, 3, new QTableWidgetItem(u.enabled ? T("Активен") : T("Отключён")));
        if (!u.enabled) {
            for (int col = 0; col < m_users->columnCount(); ++col)
                m_users->item(row, col)->setForeground(QBrush(QColor("#999999")));
        }
    }
}

// ---------------------------------------------------------------- резервная копия

QWidget *SettingsPage::buildBackupTab()
{
    auto *info = new QLabel(T("В резервную копию входят камеры, настройки программы, шаблоны и расположение камер в окне. "
                              "Пользователи и пароли входа в копию не попадают.\n\n"
                              "Внимание: адреса камер содержат их пароли. Храните файл копии в безопасном месте."), this);
    info->setWordWrap(true);

    auto *exportBtn = new QPushButton(T("Создать резервную копию…"), this);
    auto *importBtn = new QPushButton(T("Восстановить из копии…"), this);

    connect(exportBtn, &QPushButton::clicked, this, [this] {
        const QString def = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                            + QStringLiteral("/PSVMS-backup.json");
        const QString path = QFileDialog::getSaveFileName(this, T("Резервная копия"), def, QStringLiteral("JSON (*.json)"));
        if (path.isEmpty())
            return;
        QString err;
        if (Backup::exportTo(path, &err))
            QMessageBox::information(this, T("Резервная копия"), T("Резервная копия сохранена:\n%1").arg(path));
        else
            QMessageBox::warning(this, T("Резервная копия"), err);
    });
    connect(importBtn, &QPushButton::clicked, this, [this] {
        const QString path = QFileDialog::getOpenFileName(this, T("Восстановить из копии"), QString(), QStringLiteral("JSON (*.json)"));
        if (path.isEmpty())
            return;
        QMessageBox box(QMessageBox::Question, T("Восстановление"),
                        T("Текущие камеры и настройки будут заменены данными из копии, программа перезапустится. Продолжить?"),
                        QMessageBox::NoButton, this);
        QPushButton *yes = box.addButton(T("Восстановить"), QMessageBox::AcceptRole);
        box.addButton(T("Отмена"), QMessageBox::RejectRole);
        box.exec();
        if (box.clickedButton() != yes)
            return;
        QString err;
        if (!Backup::importFrom(path, &err)) {
            QMessageBox::warning(this, T("Восстановление"), err);
            return;
        }
        QProcess::startDetached(QCoreApplication::applicationFilePath(), QCoreApplication::arguments().mid(1));
        QCoreApplication::quit();
    });

    auto *buttons = new QHBoxLayout;
    buttons->addWidget(exportBtn);
    buttons->addWidget(importBtn);
    buttons->addStretch(1);

    auto *lay = new QVBoxLayout;
    lay->addWidget(info);
    lay->addLayout(buttons);
    lay->addStretch(1);
    return settingsPane(lay);
}

// ---------------------------------------------------------------- о программе

// ---------------------------------------------------------------- лицензия

QWidget *SettingsPage::buildLicenseTab()
{
    auto *editionTitle = new QLabel(this);
    editionTitle->setStyleSheet(QStringLiteral("font-size:22px;font-weight:900;"));
    auto *editionStatus = new QLabel(this);
    editionStatus->setWordWrap(true);
    auto *machineLabel = new QLabel(this);
    machineLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    machineLabel->setWordWrap(true);

    auto refresh = [editionTitle, editionStatus, machineLabel] {
        QString status;
        QDateTime expiry;
        const bool active = psvmsLicenseActive(&status, &expiry);
        editionTitle->setText(active ? T("PSVMS — полная Pro-версия") : T("PSVMS — DEMO ВЕРСИЯ"));
        editionTitle->setStyleSheet(active
            ? QStringLiteral("font-size:22px;font-weight:900;color:#e5c45a;")
            : QStringLiteral("font-size:23px;font-weight:900;color:#ff4548;letter-spacing:0.5px;"));
        editionStatus->setText(status + (active
            ? T("\nПолный доступ к функциям программы включён.")
            : T("\nДемо-режим: доступно не более 10 камер.")));
        machineLabel->setText(T("ID этого компьютера для выпуска лицензии:\n%1")
                              .arg(psvmsMachineFingerprint()));
    };
    refresh();

    auto *copyMachineBtn = new QPushButton(T("Скопировать ID компьютера"), this);
    auto *activateBtn = new QPushButton(T("Активировать файл лицензии…"), this);
    auto *removeBtn = new QPushButton(T("Удалить лицензию"), this);
    connect(copyMachineBtn, &QPushButton::clicked, this, [this] {
        QApplication::clipboard()->setText(psvmsMachineFingerprint());
        QMessageBox::information(this, T("ID компьютера"), T("ID компьютера скопирован. Отправьте его для выпуска лицензии."));
    });
    connect(activateBtn, &QPushButton::clicked, this, [this, refresh] {
        const QString path = QFileDialog::getOpenFileName(this, T("Выберите файл лицензии PSVMS"),
                                                          QString(), T("Лицензии PSVMS (*.lic);;Все файлы (*.*)"));
        if (path.isEmpty()) return;
        QString error;
        if (!psvmsImportLicense(path, &error)) {
            QMessageBox::warning(this, T("Лицензия не активирована"), error);
            return;
        }
        refresh();
        QMessageBox::information(this, T("Лицензия активирована"),
                                 T("Подпись проверена. Полная версия PSVMS активирована на этом компьютере."));
        if (auto *tabs = findChild<QTabWidget *>()) {
            for (int i = 1; i <= 4; ++i) tabs->setTabEnabled(i, true);
        }
    });
    connect(removeBtn, &QPushButton::clicked, this, [this, refresh] {
        if (QMessageBox::question(this, T("Удалить лицензию"),
                                  T("Удалить лицензию с этого компьютера? Программа вернётся в демо-режим.")) != QMessageBox::Yes)
            return;
        QSettings settings;
        settings.remove(QStringLiteral("license"));
        refresh();
        if (m_mgr) m_mgr->enforceDemoLimit();
        if (auto *tabs = findChild<QTabWidget *>()) {
            for (int i = 1; i <= 4; ++i) tabs->setTabEnabled(i, false);
        }
    });

    auto *info = new QLabel(T("Лицензия привязывается к ID компьютера. Передайте этот ID разработчику, "
                              "получите файл .lic и загрузите его здесь. Подпись проверяется встроенным "
                              "открытым ключом; срок лицензии нельзя изменить редактированием файла. "
                              "Для лицензии на срок в днях отсчёт начинается при активации."), this);
    info->setWordWrap(true);

    auto *buttons = new QHBoxLayout;
    buttons->addWidget(copyMachineBtn);
    buttons->addWidget(activateBtn);
    buttons->addWidget(removeBtn);
    buttons->addStretch(1);

    auto *lay = new QVBoxLayout;
    lay->addWidget(editionTitle);
    lay->addWidget(editionStatus);
    lay->addWidget(machineLabel);
    lay->addWidget(info);
    lay->addLayout(buttons);
    lay->addStretch(1);
    return settingsPane(lay);
}

QWidget *SettingsPage::buildAboutTab()
{
    auto *name = new QLabel(App::title(), this);
    name->setStyleSheet(QStringLiteral("font-size: 28px; font-weight: 700; color: #f2c200;"));
    auto *ver = new QLabel(T("Версия %1").arg(QString::fromLatin1(App::kVersion)), this);
    ver->setStyleSheet(QStringLiteral("color:#a8a8ac;"));
    auto *author = new QLabel(T("Разработчик: %1").arg(QString::fromUtf8(App::kAuthor)), this);
    author->setStyleSheet(QStringLiteral("color:#e6e6e6; margin-top: 18px;"));
    auto *lay = new QVBoxLayout;
    lay->addWidget(name);
    lay->addWidget(ver);
    lay->addWidget(author);
    lay->addStretch(1);
    return settingsPane(lay);
}

// ======================================================================
// app/side_nav.cpp
// ======================================================================
namespace {

constexpr int kNavExpandedWidth = 232;
constexpr int kNavCollapsedWidth = 68;

// Векторные значки ячеек: единая тонкая золотисто-светлая графика PSVMS.
QIcon makeCellIcon(const QString &kind, const QColor &color)
{
    QPixmap pm(64, 64);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen(color, 4.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    if (kind == QStringLiteral("play")) {
        p.setPen(Qt::NoPen);
        p.setBrush(color);
        QPolygonF tri;
        tri << QPointF(23, 15) << QPointF(49, 32) << QPointF(23, 49);
        p.drawPolygon(tri);
    } else if (kind == QStringLiteral("stop")) {
        p.setPen(Qt::NoPen);
        p.setBrush(color);
        p.drawRoundedRect(QRectF(18, 18, 28, 28), 5, 5);
    } else if (kind == QStringLiteral("close")) {
        p.drawLine(21, 21, 43, 43);
        p.drawLine(43, 21, 21, 43);
    } else if (kind == QStringLiteral("plus")) {
        p.drawLine(32, 17, 32, 47);
        p.drawLine(17, 32, 47, 32);
    } else if (kind == QStringLiteral("mute") || kind == QStringLiteral("sound")) {
        QPolygonF body;
        body << QPointF(12,25) << QPointF(22,25) << QPointF(35,15)
             << QPointF(35,49) << QPointF(22,39) << QPointF(12,39);
        p.setBrush(color);
        p.drawPolygon(body);
        if (kind == QStringLiteral("sound")) {
            p.setBrush(Qt::NoBrush);
            p.drawArc(QRectF(29,18,23,28), -55*16, 110*16);
            p.drawArc(QRectF(29,10,32,44), -55*16, 110*16);
        } else {
            p.drawLine(42, 22, 55, 42);
            p.drawLine(55, 22, 42, 42);
        }
    }
    return QIcon(pm);
}

// Большая эмблема пустой ячейки рисуется векторно: не зависит от внешних PNG,
// сохраняет чёткость при любом размере и использует полупрозрачное золото PSVMS.
QPixmap makeCameraCellArtwork(const QSize &requestedSize)
{
    const QSize size(qMax(1, requestedSize.width()), qMax(1, requestedSize.height()));
    QPixmap pm(size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    p.scale(size.width() / 128.0, size.height() / 128.0);

    // Мягкий ореол и тонкие концентрические контуры.
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(224, 184, 65, 13));
    p.drawEllipse(QRectF(5, 5, 118, 118));
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(232, 194, 83, 48), 1.4));
    p.drawEllipse(QRectF(10, 10, 108, 108));
    p.setPen(QPen(QColor(242, 207, 104, 105), 1.2));
    p.drawArc(QRectF(17, 17, 94, 94), 25 * 16, 285 * 16);

    // Корпус камеры и объектив — чистая геометрия без мелких деталей.
    QPainterPath body;
    body.addRoundedRect(QRectF(25, 43, 78, 49), 11, 11);
    p.setBrush(QColor(227, 190, 76, 24));
    p.setPen(QPen(QColor(245, 215, 128, 190), 3.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawPath(body);
    QPainterPath hood;
    hood.moveTo(39, 43); hood.lineTo(47, 32); hood.lineTo(76, 32); hood.lineTo(85, 43);
    p.setBrush(QColor(242, 207, 104, 22));
    p.drawPath(hood);
    p.setBrush(QColor(226, 185, 62, 34));
    p.setPen(QPen(QColor(255, 225, 145, 205), 2.6));
    p.drawEllipse(QRectF(47, 52, 34, 34));
    p.setPen(QPen(QColor(255, 220, 116, 155), 1.6));
    p.drawEllipse(QRectF(54, 59, 20, 20));
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 230, 155, 175));
    p.drawEllipse(QRectF(61, 66, 6, 6));

    // Небольшой индикатор состояния и опорная линия.
    p.setBrush(QColor(242, 197, 65, 190));
    p.drawEllipse(QRectF(88, 52, 5, 5));
    p.setPen(QPen(QColor(232, 194, 83, 90), 1.5, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(43, 101), QPointF(85, 101));
    return pm;
}

// Единый фирменный значок приложения: используется в заголовке окна и на панели задач.
static QIcon makeAppIcon()
{
    QPixmap pm(128, 128);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#171d24"));
    p.drawRoundedRect(QRectF(5, 5, 118, 118), 27, 27);
    p.setPen(QPen(QColor("#e8bd32"), 7, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(QRectF(23, 29, 82, 57), 10, 10);
    p.drawLine(64, 87, 64, 101);
    p.drawLine(45, 103, 83, 103);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#e8bd32"));
    p.drawEllipse(QRectF(56, 48, 16, 16));
    p.setPen(QPen(QColor("#e8bd32"), 4, Qt::SolidLine, Qt::RoundCap));
    p.drawArc(QRectF(42, 38, 44, 38), 25 * 16, 130 * 16);
    return QIcon(pm);
}

// круглый значок пользователя с первой буквой имени
QIcon makeAvatarIcon(const QString &name)
{
    const int size = 72;   // рисуем крупнее для чёткости на экранах с высоким DPI
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0xf2, 0xc2, 0x00));
    p.drawEllipse(0, 0, size, size);
    QFont f = p.font();
    f.setBold(true);
    f.setPixelSize(32);
    p.setFont(f);
    p.setPen(QColor(0x1a, 0x1a, 0x1c));
    p.drawText(QRect(0, 0, size, size), Qt::AlignCenter, name.isEmpty() ? QStringLiteral("?") : name.left(1).toUpper());
    return QIcon(pm);
}

} // namespace

// Фоновое изображение PSVMS встроено прямо в исходник, поэтому отдельный .qrc/файл не нужен.
static QPixmap makePsvmsBrandWallpaper()
{
    static const QByteArray encoded = QByteArrayLiteral(
        "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"
        "w6Y6UCHTGXPbTqOqSstr05bNN9OgjozdI57j105TLFi5OkpOM3H73DKCapFsY6GhTK4zicuuyJCNsaVylGmLhLVCUbFCcCI4S3Td"
        "Ur15OhjXpfqXzf6t8n3fnjn+i4X0/Hi6GD3W8+06PzXsfO9fe43rbeevm3I+y3def53j+kOb35/B19k5HXHzA9f5zvzyShLU068O"
        "rnrp9fz3R83f1P035J9N8vb4vwOnyfXwqgl6uBTpzWNNWMAVldkqjJCjOFgmimcL6r03bky9enp2XY8+st1ZrrOnm14DJg28yWvP"
        "3eOc2vbnl3Ll6DnXV2nVtots2TyXEOvxembrcE06cMkS+quBZTXlXXXz8x09fm857h+AS+38QSiGrPIcq1TctKZX2+oeRl7zonzK"
        "H2ToHxDf9u0Hxjo/VsieG2e8Z47odadcvL2InneFb4iXv+J6GGXtAuHWSjOrtOIKohZp9B5uWdV5fWcLWcF9UNZoq16l5dXZyGN7"
        "6zMWzspvjE2OrqG7J0IxiuO1NYvb6O55u+j5tk5Jinbq6Y5tXoMu8Z45btZy5+th1MrVliHA1uCMEZQI78TTp7eXprZLALvMMstP"
        "D6XKq5V3VCnUhdTndOKqb4HGcwiRkEossrtoBpkIziIcpXXdnJ7MWjOvU/ZPgP1T5Xu8N5nq8H2+aH1f5N9Yzd/l/P8ApuXTZ675"
        "l7vj06r5c/P16l/J0HTeBSb44LlzcP0768/lvmvr/wAa+l5Nmvl6OmfV/QvnHsPn+n5Pjso+j5a0R6YtrsRWBYmATg5QkhKSIKSs"
        "p6PO12dGrPbZp0ZbTRbmma55WdSOaktwX0E9HN3nMq6Oo87D0954a5EvrTh5bPV83gxl7G7yyPT08As7Wfm2S21KA07zMaaytPcc"
        "99zUeZftPEFntPFfUrMevffZVvlpsq0W2lM7bJa7bZlOfofPz3Xy6njS+l0+ED3nf+WyPsnV+Gds63gPa+bMPD9P5SXvx0R8/eq6"
        "CSdLlVRfVZUIsl2uI5erw+j2U8meo5mpk2YaD2/EydmXl6e+zzdHvkeC6Xc6B4roew6eNcf0HN8Rz6e0+a5qN462HOtSd9V+syz5"
        "o6mq6i+yWbVlso6WHpGGqYaYuyuYuxSnOj2rjjbXrMhtmc3k+l81LbU9BRbGk15wDdzuvWc1YzEk4GwJJykLAqlKcUrSS5paZS5a"
        "dlNlMpwq76H829f5e/N8v7Hx3bnD1nk+7rNHuPPW+bt2N/ks+dfQrfn1vPf0CfgN+Ne1s810eW+zbzXy6dIw2I/iv2n4j9Hx325D"
        "2+b3fUp5/wA72eHjM+h5Ki+VVrRbjXPXQrsxrZGzMr1ZBWIgpoqViszas3a1Mc+i7MU62aJ5Q3HMqPRUcjMdynio6VWEl25YMTAi"
        "kwW2Rgn3efWVdaKYdtG4wbuejpctVC6vI70uHFoxC73H0FmWFKyIsn9U+Udu5+pz+XarPqnzm7gH3Ph/PuTL99r+ZQT6rd8Z9Ivt"
        "PivU4hbGdhjtuuKujwewdLLb6g8JHuecTHwety5r09WqryenOXyszw01aijKNlSsjZCM42IFZo73nHL7j0nx6qX62vnXa569Z3Pn"
        "NmN/UofMubvH2/znx2rU+g+a4Mt5i1eVmeFTsqlczsjkNEVOxaMt9aoa6ijZhBK3OdbNfiM0M04s05oL6P0vmPU2U2qJR4v0Xl4s"
        "UILpWO00zy1x0er5rsG6vGSs49cvcv8AOyl9FZ5icvqp+UUeyu8QR9AfgA+g3fOJS+/w+Lrj0/I50dyfX4ezU9T8/wDpnzzlvJJL"
        "18PZdf5tPzdvWek+Zas36FLwOnl09tX5jVnXYxLbHHl3th5jvrgmXz9U/f5LK3os+peG+i/H/B611eHP2+b2e/59Plv6Np+ZSr6R"
        "n+eqz39PhFZ7WHjFHrq/KxX1VfmSvRU8FWdW/wA8az3s/JnW+uq2yt6Nac6XoPpVnyHV9qdfD6PX82zhV9HpHE20TN13nMxjIxzr"
        "vZufbZuyxmOd/Ws869/QPOT9P6E8Dk91wjjbbegeSXseRLj5np/MgvTeeKZ6EU9vj9iyd+7inV6k+QmzPzuwve01WnNwvjGvBAL8"
        "8Mhusokauz5nonsOj5DvHL4+vzxPz/T5cvsVoj4fXnr0VakK7a9ZgrY6ldOtXOSOmjUgpRsHF0JziguqqVV9KJxZcooldnCyEXUL"
        "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"
        "4jU6xKDS5FR7mTZAP0U1LK+nEMgM7JBHL5Hul1MQvq4TybvYtcmh0sjkEUN0e/wLXFi7a9m6T2t0BWTu/X8HcZjLvl3XwRbphmdB"
        "I17ZGT0rK5SRPikzsrKyAUVPLPLR4YyjE8sFHBVTy1dSegC+XtyGR6OzAj6FO77VTuaFl5KkgPlqHyNo5NIiuj2ma6N3O62ye5F2"
        "z3ErdU9SNJN07dzh5H7PhP3tS1km6NwrrULasmEcSUaX3zuoKzapp3RIbp3cbqya8tUxvMof4UU1pKZBHqDQBK9tm8zO1tNEELEl"
        "5aBJ5pnxccvBLuzinAPRpgvuxpzg5co5z+RrVyFWVJhbWYeRvfdrtTWuUcqjkVHWNcyvoHUkjwAnDZwRzsj3GnQUywWtt9QtIfIC"
        "ne7sgbm6ksR5dD7ZWy+eKQ35eQmPcAQoQNU28tzanJ4LO9VybqapvrsrmxJvrffutItPHA2Fb5xujT7F6CsLDdfNsiLIZO79fwiL"
        "HPuM+4RFumGV8MkMsc7JGRVUdVQTUistK0rStCosMqKxUtHFSxV1TBh7KiWWecnoAuuy7ZDcle1ubRdHzO7D0IP45zvSyaUbucxo"
        "VM3eMbaFNGODNE5sherohEIjJlUWhjw8OfZr93xfyk5NNi9+o/OZTjxKO61ZHKmqjEaila1p3cQANSupL8VQbx2N9NgGyOTpHQPE"
        "j5g1jWIbDUXLVpDZAGvdtqKcLIPLk5PYbWNiVpbrkr7yfVZQfqlQpMRllpT3UbrPTXKN6ZJdtHWMkixChfRzEJwRatKIycF8FNCs"
        "FqTn7IhW2TfMinNKDPKmyaV73EIMLlbZBE+WIp3u0u0QyvbSROkaySQmOWxlyjidLJ9Ol1Rw61FEJFLStjpxuqcU3DjZG5P03ZDT"
        "kSRxc5ygcbb8D7Rha1M4fFfwcjcoDJ3frGXub1HLvlbpaXNfTVsciaXMM2ExzJ8T4ZAy6p6aWeWhwGJqFL5a7FGwKV9y8o52yO2f"
        "YAbdzmdg1O9GN3kDNSa0NQKbcupItMbArqd9gGse2ppFwy3Io5FNe5hErXRu90X8vzdAoL56KY7vGiRA+VOV1HUyRJlG+anGHbjD"
        "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"
        "VMlhNUp9j/vdX+aZZJzTQOd+RV5ovQTg7l3HTw0ZKWjJ0S4zsMp9R46UmRy9vYiawVzK2Zkn+JizuSuEOfrQ6tKnFh3mdttQr0F1"
        "2fkVciEQX9+6f6K69sjY5kRt5rhIg8U/JiZ5irHc4ZP6dQkek4HtWfe/PTFR4KUgNGgmqyHiqv8AJesVufFbjVut8lut8luNW6Fu"
        "/FcVj9X0Xf1L7359Rm7BTboSXzTLNaGaBzvyKvN1oBwPZ7jsKqX1nfyeKYZ+BWby1ohHtjdKzuSlsJxrq1Y5ZvKYV3lyPgs1Ebn8"
        "n7Jxb+EoRYD78P4juNhd90j2dRl/qf8AlZ9789AGYrwQEhTQwWs+f4VqsHtqqu+vKLv6l978+oSOCppfM8sk6E6gc7h3FZxmtAOB"
        "7PcdCmhNS+qMFqlYTVRLQro+hdNnGE7A/wBEWFod2oT8QjFgziQfi3xWfycyPFvB3is7CoRR7Di0qfUv9z/ys+9+avSpzQaGDx0N"
        "c3fzWq32lVP2B7+pSdvc+pTGj8zyzWgOoC7h3HuV9mtk7sD2e7Z0hla8aG32zW893skqQh7T9S6zZqbFW3u0REhvLHjAhZt4DYwF"
        "W8HeCOU5O30XrN7H9kMqgeDm8HDkoeVQTOHEqP6I9Sp0n/ki18nEciqm2bzdHxUoYu9/H7C9/UpE15qR6hSyWGh8zyyToDqAu4dx"
        "7lnIWtk549nR1YL/ACXpY0GH4umqxokT8DZL0eSz74jpqTSGD7oktd7j4n6mnyXsmhOhOAdQ2V0LuiHMcWuFQ4cFMgZxtIjVqD0L"
        "93u7lE+Tn4RNeF3P5e1HqQzRJ+9bMbvaK9HU9oqvW8fqzv6ldf7CpHqskMkyjJokTJjQPc3d/sjdy3J2w+TnVb3KseNGP+2yX5r0"
        "WQT74r5qUPNQR/tskvSRnu8T9UYrpGqlfBSI81h4dyzhjGNqEDO1kgx0N3jOelPRbF9U6r/BPg8TVp7+CDm0e0z8Cm5QzcjsETwP"
        "H49Qc4Sk2pmmsutAbytvRvYwYlSNG8AMB9nLj/YeSkdvLFTbk8TxkvSxoMH8Tl6TLHxTyhMXosgL++M9SgCDA/7bF6bKoz/F2lul"
        "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"
        "zmc+jqGo8QW6nEqUx3cMTubcztLalsE//9k="
    );
    static const QPixmap wallpaper = [] {
        QPixmap pm;
        pm.loadFromData(QByteArray::fromBase64(encoded), "JPG");
        return pm;
    }();
    return wallpaper;
}

SideNav::SideNav(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("sideNav"));
    setAttribute(Qt::WA_StyledBackground, true);
    m_wallpaper = makePsvmsBrandWallpaper();
    setFixedWidth(kNavExpandedWidth);

    // Подсветка создаётся первой, поэтому лежит под кнопками меню
    m_pill = new QFrame(this);
    m_pill->setObjectName(QStringLiteral("navPill"));
    m_pill->hide();

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(10, 12, 10, 12);
    root->setSpacing(4);

    // Шапка: компактная кнопка с анимированным сворачиванием навигации, логотип и бренд.
    auto *head = new QHBoxLayout;
    head->setContentsMargins(0, 0, 0, 0);
    m_toggleBtn = new QToolButton(this);
    m_toggleBtn->setObjectName(QStringLiteral("navToggle"));
    m_toggleBtn->setText(QStringLiteral("‹"));
    m_toggleBtn->setToolTip(T("Свернуть боковую панель"));
    m_toggleBtn->setCursor(Qt::PointingHandCursor);
    m_toggleBtn->setFixedSize(32, 32);
    m_toggleBtn->setStyleSheet(QStringLiteral(
        "QToolButton{color:#f1cf65;background:rgba(13,18,23,150);border:1px solid rgba(222,184,72,115);"
        "border-radius:10px;font-size:27px;font-weight:500;padding:0;} "
        "QToolButton:hover{color:#fff1b4;background:rgba(222,184,72,35);border-color:#f1cf65;}"));
    m_brandIcon = new QLabel(this);
    // Увеличенный фирменный знак в шапке боковой панели.
    m_brandIcon->setFixedSize(54, 54);
    m_brandIcon->setAlignment(Qt::AlignCenter);
    m_brandIcon->setPixmap(QPixmap(QStringLiteral(":/branding/psvms.png")).scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_brandIcon->setToolTip(App::title());
    m_brand = new QLabel(QStringLiteral("PSVMS"), this);
    m_brand->setObjectName(QStringLiteral("navBrand"));
    m_brand->setStyleSheet(QStringLiteral("font-size:20px;font-weight:900;letter-spacing:1.4px;color:#f2d36b;"));
    m_brand->setToolTip(App::title());
    head->addWidget(m_brandIcon);
    head->addWidget(m_brand);
    head->addStretch(1);
    head->addWidget(m_toggleBtn, 0, Qt::AlignTop);
    connect(m_toggleBtn, &QToolButton::clicked, this, [this] { setExpanded(!m_expanded, true); });
    root->addLayout(head);
    root->addSpacing(18);

    m_itemsLayout = new QVBoxLayout;
    m_itemsLayout->setContentsMargins(0, 0, 0, 0);
    m_itemsLayout->setSpacing(4);
    root->addLayout(m_itemsLayout);
    root->addStretch(1);

    m_bottomLayout = new QVBoxLayout;
    m_bottomLayout->setContentsMargins(0, 0, 0, 0);
    m_bottomLayout->setSpacing(2);
    root->addLayout(m_bottomLayout);
    root->addSpacing(8);

    // низ: учётная запись
    m_userBtn = new QPushButton(this);
    m_userBtn->setObjectName(QStringLiteral("navUserBtn"));
    m_userBtn->setCursor(Qt::PointingHandCursor);
    m_userBtn->setFocusPolicy(Qt::NoFocus);
    m_userBtn->setFixedHeight(52);
    m_userBtn->setIconSize(QSize(36, 36));
    root->addWidget(m_userBtn);

    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);
    connect(m_group, &QButtonGroup::idClicked, this, [this](int i) {
        if (i >= 0 && i < m_items.size()) {
            if (m_items.at(i).isPage)
                setCurrent(i, true);
            else
                emit actionTriggered(i);
        }
    });
    connect(m_userBtn, &QPushButton::clicked, this, [this] { showUserMenu(); });

    applyTexts();
}

// Единый набор контурных иконок основного меню: одинаковый холст и толщина линий.
static QIcon makeNavigationIcon(const QString &kind)
{
    QPixmap pm(40, 40);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    const QColor gold("#e8bd32");
    const QColor light("#d7dce1");
    p.setPen(QPen(gold, 2.25, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
    if (kind == QStringLiteral("▣")) { // мониторинг: экран с четырьмя каналами
        p.drawRoundedRect(QRectF(4.5, 6, 31, 23), 3, 3);
        p.drawLine(20, 29, 20, 34);
        p.drawLine(13, 35, 27, 35);
        p.drawLine(20, 29, 20, 34);
        p.drawLine(20, 17, 20, 26);
        p.drawLine(9, 17, 31, 17);
        p.drawRect(QRectF(9, 11, 8, 5));
        p.drawRect(QRectF(23, 11, 8, 5));
        p.drawRect(QRectF(9, 19, 8, 6));
        p.drawRect(QRectF(23, 19, 8, 6));
    } else if (kind == QStringLiteral("▤")) { // устройства: купольная IP-камера
        p.drawRoundedRect(QRectF(5, 7, 30, 12), 5, 5);
        p.drawArc(QRectF(8, 13, 24, 22), 180 * 16, 180 * 16);
        p.drawEllipse(QRectF(15, 20, 10, 10));
        p.setPen(QPen(light, 1.5));
        p.drawEllipse(QRectF(18, 23, 4, 4));
        p.setPen(QPen(gold, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.drawLine(11, 20, 8, 24);
        p.drawLine(29, 20, 32, 24);
    } else if (kind == QStringLiteral("▧")) { // архив
        p.drawRoundedRect(QRectF(5, 8, 30, 26), 4, 4);
        p.drawLine(5, 15, 35, 15);
        p.drawLine(15, 20, 25, 20);
        p.drawLine(15, 25, 25, 25);
    } else if (kind == QStringLiteral("♟")) { // пользователи
        p.drawEllipse(QRectF(13, 5, 14, 14));
        p.drawArc(QRectF(5, 18, 30, 24), 0, 180 * 16);
        p.drawEllipse(QRectF(3, 12, 9, 9));
        p.drawEllipse(QRectF(28, 12, 9, 9));
    } else if (kind == QStringLiteral("⚙")) { // настройки
        p.drawEllipse(QRectF(11, 11, 18, 18));
        p.drawEllipse(QRectF(17, 17, 6, 6));
        for (int i = 0; i < 8; ++i) {
            p.save(); p.translate(20, 20); p.rotate(i * 45.0);
            p.drawRoundedRect(QRectF(-2.5, -19, 5, 7), 1.5, 1.5); p.restore();
        }
    } else if (kind == QStringLiteral("ⓘ")) { // о программе
        p.drawEllipse(QRectF(4, 4, 32, 32));
        p.setPen(QPen(light, 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.drawEllipse(QRectF(18, 11, 4, 4));
        p.drawLine(20, 19, 20, 29);
    } else { // события: журнал с отметкой времени
        p.drawRoundedRect(QRectF(7, 5, 26, 30), 3, 3);
        p.drawLine(13, 4, 13, 10);
        p.drawLine(27, 4, 27, 10);
        p.drawLine(8, 13, 32, 13);
        p.drawLine(12, 19, 18, 19);
        p.drawLine(12, 24, 18, 24);
        p.drawLine(12, 29, 18, 29);
        p.drawEllipse(QRectF(21, 19, 9, 9));
        p.drawLine(25.5, 21, 25.5, 24);
        p.drawLine(25.5, 24, 28, 25.5);
    }
    return QIcon(pm);
}

static QIcon makeCameraStatusIcon()
{
    QPixmap pm(32, 32);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QPen(QColor("#f2c200"), 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QRectF(2, 2, 27, 27));
    p.drawEllipse(QRectF(9, 9, 13, 13));
    p.setPen(QPen(QColor("#55c878"), 3.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawLine(17, 23, 21, 27);
    p.drawLine(21, 27, 30, 17);
    return QIcon(pm);
}

int SideNav::addItem(const QString &icon, const QString &title, bool isPage)
{
    auto *b = new QPushButton(this);
    b->setObjectName(QStringLiteral("navItem"));
    b->setCheckable(isPage);
    b->setCursor(Qt::PointingHandCursor);
    b->setIcon(makeNavigationIcon(icon));
    b->setIconSize(QSize(28, 28));
    b->setFocusPolicy(Qt::NoFocus);
    b->setFixedHeight(44);
    b->installEventFilter(this);   // следим за положением кнопки, чтобы подсветка не отставала

    const int idx = int(m_items.size());
    Item it;
    it.icon = icon;
    it.title = title;
    it.button = b;
    it.isPage = isPage;
    m_items.append(it);
    m_group->addButton(b, idx);
    m_itemsLayout->addWidget(b);
    applyTexts();
    return idx;
}

// Фирменные монохромные значки нижних пунктов меню: графит + янтарное золото.
static QIcon makeSideActionIcon(bool language)
{
    QPixmap pm(40, 40);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(QColor("#f2c200"), 2.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    if (language) {
        p.drawEllipse(QRectF(5, 5, 30, 30));
        p.drawEllipse(QRectF(13, 5, 14, 30));
        p.drawArc(QRectF(7, 12, 26, 16), 0, 180 * 16);
        p.drawArc(QRectF(7, 12, 26, 16), 180 * 16, 180 * 16);
        p.drawLine(6, 20, 34, 20);
    } else {
        const QPointF c(20, 20);
        for (int i = 0; i < 8; ++i) {
            p.save(); p.translate(c); p.rotate(i * 45.0);
            p.drawRoundedRect(QRectF(-3.0, -17.0, 6.0, 9.0), 1.5, 1.5);
            p.restore();
        }
        p.drawEllipse(QRectF(8, 8, 24, 24));
        p.setPen(QPen(QColor("#f2c200"), 2.0));
        p.drawEllipse(QRectF(15, 15, 10, 10));
    }
    return QIcon(pm);
}

// Нижние действия сделаны крупнее; настройки и язык выделены фирменными векторными значками.
int SideNav::addBottomAction(const QString &icon, const QString &title)
{
    auto *b = new QPushButton(this);
    b->setObjectName(QStringLiteral("navBottomItem"));
    b->setCursor(Qt::PointingHandCursor);
    b->setFocusPolicy(Qt::NoFocus);
    const bool prominent = (icon == QStringLiteral("⚙") || icon == QStringLiteral("🌐"));
    b->setProperty("prominent", prominent);
    b->setFixedHeight(prominent ? 52 : 40);
    if (prominent) {
        b->setIcon(makeSideActionIcon(icon == QStringLiteral("🌐")));
        b->setIconSize(QSize(28, 28));
    } else {
        b->setIconSize(QSize(22, 22));
    }

    const int idx = int(m_items.size());
    Item it;
    it.icon = icon;
    it.title = title;
    it.button = b;
    it.isPage = false;
    m_items.append(it);
    m_bottomLayout->addWidget(b);
    connect(b, &QPushButton::clicked, this, [this, idx] { emit actionTriggered(idx); });
    applyTexts();
    return idx;
}

int SideNav::addPage(const QString &icon, const QString &title)
{
    return addItem(icon, title, true);
}

int SideNav::addAction(const QString &icon, const QString &title)
{
    const int idx = addItem(icon, title, false);
    // пункт-действие не должен участвовать в выборе страницы
    m_group->removeButton(m_items.at(idx).button);
    connect(m_items.at(idx).button, &QPushButton::clicked, this, [this, idx] { emit actionTriggered(idx); });
    return idx;
}

void SideNav::setItemText(int index, const QString &icon, const QString &title)
{
    if (index < 0 || index >= m_items.size())
        return;
    m_items[index].icon = icon;
    m_items[index].title = title;
    applyTexts();
}

void SideNav::setItemVisible(int index, bool visible)
{
    if (index >= 0 && index < m_items.size())
        m_items.at(index).button->setVisible(visible);
}

QWidget *SideNav::itemButton(int index) const
{
    return (index >= 0 && index < m_items.size()) ? m_items.at(index).button : nullptr;
}

void SideNav::setCurrent(int index, bool animate)
{
    if (index < 0 || index >= m_items.size() || !m_items.at(index).isPage)
        return;
    const bool changed = index != m_current;
    m_current = index;
    m_items.at(index).button->setChecked(true);
    updatePill(animate && changed && Cfg::view().animations);
    if (changed)
        emit currentChanged(index);
}

void SideNav::setUser(const QString &name, const QString &role)
{
    m_userName = name;
    m_userRole = role;
    m_userBtn->setIcon(makeAvatarIcon(name));
    applyTexts();
}

void SideNav::showUserMenu()
{
    QMenu menu(this);
    QAction *title = menu.addAction(m_userName + QStringLiteral(" — ") + m_userRole);
    title->setEnabled(false);
    menu.addSeparator();
    QAction *pw = menu.addAction(T("Сменить пароль…"));
    QAction *out = menu.addAction(T("Сменить пользователя…"));
    connect(pw, &QAction::triggered, this, [this] { emit changePasswordRequested(); });
    connect(out, &QAction::triggered, this, [this] { emit logoutRequested(); });
    menu.exec(m_userBtn->mapToGlobal(QPoint(0, -menu.sizeHint().height())));
}

void SideNav::setNavWidth(int w)
{
    setFixedWidth(w);
}

void SideNav::setExpanded(bool on, bool animate)
{
    const int target = on ? kNavExpandedWidth : kNavCollapsedWidth;
    if (on == m_expanded && width() == target)
        return;
    m_expanded = on;

    if (m_widthAnim)
        m_widthAnim->stop();
    if (!animate || !Cfg::view().animations) {
        setNavWidth(target);
        applyTexts();
        return;
    }
    if (on)
        applyTexts();   // при раскрытии названия появляются сразу, пока меню растёт

    auto *anim = new QPropertyAnimation(this, "navWidth", this);
    anim->setDuration(220);
    anim->setEasingCurve(QEasingCurve::InOutCubic);
    anim->setStartValue(width());
    anim->setEndValue(target);
    connect(anim, &QPropertyAnimation::finished, this, [this] { applyTexts(); });   // при сворачивании — после анимации
    m_widthAnim = anim;
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void SideNav::applyTexts()
{
    for (const Item &it : std::as_const(m_items)) {
        const bool prominent = it.button->property("prominent").toBool();
        if (prominent) {
            it.button->setText(m_expanded ? it.title : QString());
            it.button->setToolTip(m_expanded ? QString() : it.title);
        } else if (it.isPage) {
            it.button->setText(m_expanded ? it.title : QString());
            it.button->setToolTip(m_expanded ? QString() : it.title);
        } else if (m_expanded) {
            it.button->setText(QStringLiteral("%1     %2").arg(it.icon, it.title));
            it.button->setToolTip(QString());
        } else {
            it.button->setText(it.icon);
            it.button->setToolTip(it.title);
        }
        it.button->setProperty("collapsed", !m_expanded);
        it.button->style()->unpolish(it.button);
        it.button->style()->polish(it.button);
    }
    m_userBtn->setText(m_expanded ? QStringLiteral("  ") + m_userName : QString());
    m_userBtn->setToolTip(m_expanded ? QString() : m_userName);
    m_userBtn->setProperty("collapsed", !m_expanded);
    m_userBtn->style()->unpolish(m_userBtn);
    m_userBtn->style()->polish(m_userBtn);
    m_brand->setVisible(m_expanded);
    if (m_brandIcon) m_brandIcon->setVisible(m_expanded);
    if (m_toggleBtn) {
        m_toggleBtn->setText(m_expanded ? QStringLiteral("‹") : QStringLiteral("›"));
        m_toggleBtn->setToolTip(m_expanded ? T("Свернуть боковую панель") : T("Развернуть боковую панель"));
    }
}

void SideNav::updatePill(bool animate)
{
    if (m_current < 0 || m_current >= m_items.size())
        return;
    const QRect target = m_items.at(m_current).button->geometry();

    if (m_pillAnim)
        m_pillAnim->stop();
    if (!animate || !m_pill->isVisible()) {
        m_pill->setGeometry(target);
        m_pill->show();
        m_pill->lower();
        return;
    }
    auto *anim = new QPropertyAnimation(m_pill, "geometry", this);
    anim->setDuration(280);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setStartValue(m_pill->geometry());
    anim->setEndValue(target);
    m_pillAnim = anim;
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void SideNav::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    p.fillRect(rect(), QColor(8, 9, 11, 160));
    if (!m_wallpaper.isNull()) {
        const QSize hostSize = parentWidget() ? parentWidget()->size() : size();
        const QSize scaled = m_wallpaper.size().scaled(hostSize, Qt::KeepAspectRatioByExpanding);
        const QPixmap fitted = m_wallpaper.scaled(scaled, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        const int cropX = qBound(0, int((fitted.width()-hostSize.width())/2), qMax(0, fitted.width()-width()));
        const int cropY = qBound(0, int((fitted.height()-hostSize.height())/2), qMax(0, fitted.height()-height()));
        const QRect source(cropX, cropY, qMin(width(), fitted.width()-cropX), qMin(height(), fitted.height()-cropY));
        p.drawPixmap(rect(), fitted, source);
        p.fillRect(rect(), QColor(5, 6, 8, 108));
        QLinearGradient shade(0, 0, width(), 0);
        shade.setColorAt(0.0, QColor(7, 8, 10, 125));
        shade.setColorAt(1.0, QColor(7, 8, 10, 75));
        p.fillRect(rect(), shade);
    }
    p.setPen(QPen(QColor(224, 177, 81, 185), 1));
    p.drawLine(width()-1, 0, width()-1, height());
}

bool SideNav::eventFilter(QObject *obj, QEvent *e)
{
    // кнопка сдвинулась или изменила размер (раскладка, сворачивание меню): подсветка идёт за ней
    if ((e->type() == QEvent::Resize || e->type() == QEvent::Move) && m_current >= 0
        && m_current < m_items.size() && obj == m_items.at(m_current).button && !m_pillAnim)
        updatePill(false);
    return QWidget::eventFilter(obj, e);
}

// ======================================================================
// app/splash.cpp
// ======================================================================
static QPixmap psvmsSplashPixmap()
{
    QPixmap pixmap(1200, 675);
    pixmap.fill(QColor("#090d14"));
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing, true);

    QLinearGradient bg(0, 0, 1200, 675);
    bg.setColorAt(0.0, QColor("#101c2b"));
    bg.setColorAt(0.52, QColor("#11151e"));
    bg.setColorAt(1.0, QColor("#090b10"));
    p.fillRect(pixmap.rect(), bg);

    QRadialGradient glow(QPointF(600, 330), 520);
    glow.setColorAt(0.0, QColor(33, 91, 132, 105));
    glow.setColorAt(0.65, QColor(18, 46, 74, 48));
    glow.setColorAt(1.0, QColor(5, 10, 18, 0));
    p.fillRect(pixmap.rect(), glow);

    p.setPen(QPen(QColor(213, 171, 83, 95), 2));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(QRectF(34, 34, 1132, 607), 26, 26);

    QFont title(QStringLiteral("Segoe UI"), 58, QFont::Black);
    title.setLetterSpacing(QFont::AbsoluteSpacing, 5.0);
    p.setFont(title);
    QLinearGradient gold(0, 245, 0, 360);
    gold.setColorAt(0, QColor("#fff0bf"));
    gold.setColorAt(0.5, QColor("#e9bd61"));
    gold.setColorAt(1, QColor("#a97529"));
    p.setPen(QPen(gold, 1));
    p.drawText(QRect(70, 235, 1060, 100), Qt::AlignCenter, QStringLiteral("PSVMS"));

    QFont subtitle(QStringLiteral("Segoe UI"), 16, QFont::DemiBold);
    p.setFont(subtitle);
    p.setPen(QColor("#c6d4e2"));
    p.drawText(QRect(100, 355, 1000, 42), Qt::AlignCenter,
               QStringLiteral("PROFESSIONAL VIDEO MONITORING SYSTEM"));

    p.setPen(QPen(QColor(218, 174, 89, 130), 2));
    p.drawLine(390, 425, 810, 425);
    p.end();
    return pixmap;
}

class AnimatedSplash final : public QSplashScreen
{
public:
    AnimatedSplash()
        : QSplashScreen(psvmsSplashPixmap(),
                        Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint)
    {
        setFixedSize(pixmap().size());
        setWindowTitle(QStringLiteral("PSVMS"));
        m_timer = new QTimer(this);
        m_timer->setInterval(33);
        connect(m_timer, &QTimer::timeout, this, [this] {
            m_phase = std::fmod(m_phase + 0.035, 1.0);
            update();
        });
        if (Cfg::view().animations)
            m_timer->start();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QSplashScreen::paintEvent(event);
        if (!Cfg::view().animations)
            return;
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        const qreal pulse = 0.5 + 0.5 * std::sin(m_phase * 6.283185307179586);
        const qreal radius = 205.0 + 18.0 * pulse;
        QPen ring(QColor(104, 183, 224, 22 + int(24 * pulse)), 2.0);
        p.setPen(ring);
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(width() / 2.0, height() / 2.0 - 10), radius, radius * 0.55);
        p.setPen(QPen(QColor(232, 190, 104, 100 + int(80 * pulse)), 3.0));
        const int x = 360 + int(m_phase * 480.0);
        p.drawLine(x, 462, qMin(x + 120, 840), 462);
        p.setPen(QColor(220, 229, 238, 175));
        p.setFont(QFont(QStringLiteral("Segoe UI"), 10, QFont::DemiBold));
        p.drawText(QRect(100, 520, width() - 200, 32), Qt::AlignCenter,
                   QStringLiteral("ЗАГРУЗКА СИСТЕМЫ  •  ИНИЦИАЛИЗАЦИЯ МОДУЛЕЙ"));
    }

private:
    QTimer *m_timer = nullptr;
    qreal m_phase = 0.0;
};

QSplashScreen *createSplash()
{
    auto *splash = new AnimatedSplash;
    splash->show();
    QCoreApplication::processEvents();
    return splash;
}

void finishSplash(QSplashScreen *splash)
{
    if (!splash)
        return;
    if (Cfg::view().animations) {
        QEventLoop loop;
        auto *anim = new QPropertyAnimation(splash, "windowOpacity", splash);
        anim->setDuration(350);
        anim->setStartValue(1.0);
        anim->setEndValue(0.0);
        QObject::connect(anim, &QPropertyAnimation::finished, &loop, &QEventLoop::quit);
        anim->start();
        loop.exec();
    }
    splash->close();
    delete splash;
}

// ======================================================================
// app/status_dialog.cpp
// ======================================================================
CameraStatusDialog::CameraStatusDialog(CameraManager *mgr, QWidget *parent) : QDialog(parent), m_mgr(mgr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(T("Состояние камер"));
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    resize(860, 560);

    auto *cards = new QHBoxLayout;
    for (int i = 0; i < 4; ++i) {
        m_cards[i] = new QLabel(this);
        m_cards[i]->setObjectName(QStringLiteral("statCard"));
        m_cards[i]->setAlignment(Qt::AlignCenter);
        m_cards[i]->setTextFormat(Qt::RichText);
        m_cards[i]->setMinimumHeight(86);
        cards->addWidget(m_cards[i], 1);
    }

    m_table = new QTableWidget(0, 5, this);
    m_table->setHorizontalHeaderLabels({T("Название"), T("Группа"), T("Адрес"), T("Связь"), T("Запись")});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->setColumnWidth(0, 220);
    m_table->setColumnWidth(1, 160);
    m_table->setColumnWidth(3, 110);
    m_table->setColumnWidth(4, 200);
    m_table->setMouseTracking(true);
    m_table->viewport()->setMouseTracking(true);
    new CameraHoverPreview(m_table, m_mgr, m_table);

    auto *check = new QPushButton(T("Проверить сейчас"), this);
    auto *close = new QPushButton(T("Закрыть"), this);
    auto *bottom = new QHBoxLayout;
    bottom->addWidget(check);
    bottom->addStretch(1);
    bottom->addWidget(close);

    auto *root = new QVBoxLayout(this);
    root->addLayout(cards);
    root->addWidget(m_table, 1);
    root->addLayout(bottom);

    connect(check, &QPushButton::clicked, m_mgr, &CameraManager::probeNow);
    connect(close, &QPushButton::clicked, this, &QWidget::close);
    connect(m_mgr, &CameraManager::onlineChanged, this, [this] { rebuild(); });
    connect(m_mgr, &CameraManager::camerasChanged, this, [this] { rebuild(); });
    rebuild();
}

void CameraStatusDialog::showOfflineOnly()
{
    m_offlineOnly = true;
    setWindowTitle(T("Камеры не в сети"));
    rebuild();
    show();
    raise();
    activateWindow();
}

void CameraStatusDialog::showAllCameras()
{
    m_offlineOnly = false;
    setWindowTitle(T("Состояние камер"));
    rebuild();
    show();
    raise();
    activateWindow();
}

void CameraStatusDialog::rebuild()
{
    const int total = int(m_mgr->cameras().size());
    const struct { QString label; int value; const char *color; } cards[4] = {
        {T("Всего камер"), total, "#e6e6e6"},
        {T("В сети"), m_mgr->onlineCount(), "#3fb950"},
        {T("Не в сети"), m_mgr->offlineCount(), "#e8833a"},
        {T("Идёт запись"), m_mgr->recordingCount(), "#f2c200"},
    };
    for (int i = 0; i < 4; ++i) {
        m_cards[i]->setText(QStringLiteral("<span style='font-size:30px; font-weight:700; color:%1'>%2</span><br>"
                                           "<span style='color:#a8a8ac'>%3</span>")
                                .arg(QLatin1String(cards[i].color)).arg(cards[i].value).arg(cards[i].label));
    }

    m_table->setRowCount(0);
    for (const CameraEntry &e : m_mgr->cameras()) {
        const bool online = m_mgr->isOnline(e.id);
        if (m_offlineOnly && online)
            continue;
        const int row = m_table->rowCount();
        m_table->insertRow(row);
        const QString rec = m_mgr->isStarted(e.id) ? stateText(m_mgr->stateOf(e.id)) : T("Не запущена");
        const QString cells[5] = {e.name, e.group, QUrl(e.url).host(), online ? T("В сети") : T("Не в сети"), rec};
        for (int c = 0; c < 5; ++c) {
            auto *it = new QTableWidgetItem(cells[c]);
            if (c == 0)
                it->setData(Qt::UserRole, e.id);
            if (c == 3)
                it->setForeground(QBrush(QColor(online ? "#3fb950" : "#e8833a")));
            m_table->setItem(row, c, it);
        }
    }
}

// ======================================================================
// app/top_bar.cpp
// ======================================================================
TopBar::TopBar(CameraManager *mgr, QWidget *parent) : QWidget(parent), m_mgr(mgr)
{
    setObjectName(QStringLiteral("topBar"));
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedHeight(52);

    auto *lay = new QHBoxLayout(this);
    lay->setContentsMargins(20, 0, 14, 0);
    lay->setSpacing(8);

    m_title = new QLabel(this);
    m_title->setObjectName(QStringLiteral("topTitle"));

    m_online = new QLabel(this);
    m_online->setObjectName(QStringLiteral("chipOnline"));
    m_online->setAlignment(Qt::AlignCenter);
    m_online->setMinimumHeight(34);
    m_online->setMinimumWidth(112);
    m_online->setTextInteractionFlags(Qt::NoTextInteraction);
    m_online->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_offline = new QLabel(this);
    m_offline->setObjectName(QStringLiteral("chipOffline"));
    m_offline->setAlignment(Qt::AlignCenter);
    m_offline->setMinimumHeight(34);
    m_offline->setMinimumWidth(128);
    m_offline->setTextInteractionFlags(Qt::NoTextInteraction);
    m_offline->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    m_settings = new QPushButton(T("Настройки"), this);
    m_settings->setObjectName(QStringLiteral("topSettingsBtn"));
    m_settings->setCursor(Qt::PointingHandCursor);
    m_settings->setToolTip(T("Открыть настройки программы"));
    m_fullscreen = new QPushButton(T("⛶  Во весь экран"), this);
    m_fullscreen->setObjectName(QStringLiteral("topFullscreenBtn"));
    m_fullscreen->setCursor(Qt::PointingHandCursor);
    m_fullscreen->setToolTip(T("Переключить полноэкранный режим (F11)"));
    m_status = new QPushButton(T("Состояние камер"), this);
    m_status->setObjectName(QStringLiteral("cameraStatusBtn"));
    m_status->setIcon(makeCameraStatusIcon());
    m_status->setIconSize(QSize(24, 24));
    m_status->setMinimumHeight(36);
    m_status->setCursor(Qt::PointingHandCursor);
    m_status->setToolTip(T("Открыть подробное состояние всех камер"));

    m_cloud = new QPushButton(QStringLiteral("☁  Hik-Connect"), this);
    m_cloud->setObjectName(QStringLiteral("cloudBtn"));
    m_cloud->setCursor(Qt::PointingHandCursor);
    m_cloud->setToolTip(T("Облако Hik-Connect"));

    // Переключатель простого и расширенного режима — для слабых компьютеров; кнопка выделена цветом
    m_mode = new QPushButton(this);
    m_mode->setObjectName(QStringLiteral("modeBtn"));
    m_mode->setCursor(Qt::PointingHandCursor);
    m_mode->setToolTip(T("Простой режим снижает нагрузку: меньше кадров в секунду, без анимаций и сглаживания"));

    lay->addWidget(m_title);
    lay->addStretch(1);
    lay->addWidget(m_online);
    lay->addWidget(m_offline);
    lay->addWidget(m_settings);
    lay->addWidget(m_fullscreen);
    lay->addWidget(m_status);
    lay->addSpacing(8);
    lay->addWidget(m_mode);
    lay->addSpacing(8);
    lay->addWidget(m_cloud);

    connect(m_settings, &QPushButton::clicked, this, &TopBar::settingsRequested);
    connect(m_fullscreen, &QPushButton::clicked, this, &TopBar::fullscreenRequested);
    connect(m_status, &QPushButton::clicked, this, &TopBar::statusRequested);
    connect(m_cloud, &QPushButton::clicked, this, &TopBar::cloudRequested);
    connect(m_mode, &QPushButton::clicked, this, [this] { emit simpleModeToggled(!m_simple); });

    m_simple = QSettings().value(QStringLiteral("perf/lowPower"), false).toBool();
    updateModeButton();
    updateStatusVisibility();
    connect(Cfg::notifier(), &CfgNotifier::changed, this, [this] {
        const bool v = QSettings().value(QStringLiteral("perf/lowPower"), false).toBool();
        if (v != m_simple) {
            m_simple = v;
            updateModeButton();
        }
        updateStatusVisibility();
    });

    connect(m_mgr, &CameraManager::onlineChanged, this, &TopBar::updateCounts);
    connect(m_mgr, &CameraManager::camerasChanged, this, &TopBar::updateCounts);
    updateCounts();
}

void TopBar::setTitle(const QString &title)
{
    m_title->setText(title);
}

void TopBar::setSettingsVisible(bool visible)
{
    if (m_settings) m_settings->setVisible(visible);
}

void TopBar::updateCounts()
{
    m_online->setText(T("● В сети: %1").arg(m_mgr->onlineCount()));
    m_offline->setText(T("● Не в сети: %1").arg(m_mgr->offlineCount()));
}

void TopBar::updateStatusVisibility()
{
    const bool visible = QSettings().value(QStringLiteral("ui/showCameraStatus"), true).toBool();
    m_online->setVisible(visible);
    m_offline->setVisible(visible);
    m_status->setVisible(visible);
}

void TopBar::updateModeButton()
{
    m_mode->setText(m_simple ? T("Простой режим") : T("Расширенный режим"));
    m_mode->setProperty("active", m_simple);
    m_mode->style()->unpolish(m_mode);
    m_mode->style()->polish(m_mode);
}

// ======================================================================
// app/video_cell.cpp
// ======================================================================
VideoCell::VideoCell(QWidget *parent) : QFrame(parent)
{
    setAcceptDrops(true);
    setCursor(Qt::OpenHandCursor);

    // ---- заголовок ----
    m_bar = new QWidget(this);
    m_bar->setObjectName(QStringLiteral("cellBar"));
    auto *barLay = new QHBoxLayout(m_bar);
    barLay->setContentsMargins(0, 0, 0, 0);
    barLay->setSpacing(0);

    m_title = new QLabel(m_bar);
    m_title->setObjectName(QStringLiteral("cellTitle"));
    m_title->setCursor(Qt::OpenHandCursor);
    m_title->setToolTip(T("Перетащите заголовок камеры на другую ячейку: камеры поменяются местами; положение сохранится автоматически"));
    m_title->installEventFilter(this);

    m_audioBtn = new QToolButton(m_bar);
    m_audioBtn->setObjectName(QStringLiteral("cellBtn"));
    m_audioBtn->setToolTip(T("Включить или выключить звук"));
    m_audioBtn->setIconSize(QSize(18, 18));
    m_volumeSlider = new QSlider(Qt::Horizontal, m_bar);
    m_volumeSlider->setObjectName(QStringLiteral("cellVolume"));
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(70);
    m_volumeSlider->setMinimumWidth(38);
    m_volumeSlider->setMaximumWidth(68);
    m_volumeSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_volumeSlider->setFixedHeight(14);
    m_volumeSlider->setToolTip(T("Громкость звука камеры — можно настроить заранее"));
    m_volumeSlider->setEnabled(true);
    m_closeBtn = new QToolButton(m_bar);
    m_closeBtn->setObjectName(QStringLiteral("cellBtn"));
    m_closeBtn->setIcon(makeCellIcon(QStringLiteral("close")));
    m_closeBtn->setIconSize(QSize(18, 18));
    m_closeBtn->setToolTip(T("Закрыть камеру"));
    barLay->addWidget(m_title, 1);
    barLay->addWidget(m_audioBtn);
    barLay->addWidget(m_volumeSlider);
    barLay->addWidget(m_closeBtn);

    // ---- видео и «пустая» страница ----
    m_surface = new VideoSurface(this);

    m_idle = new QWidget(this);
    m_idle->setObjectName(QStringLiteral("cellIdle"));
    auto *idleLay = new QVBoxLayout(m_idle);
    idleLay->setContentsMargins(8, 8, 8, 8);
    m_play = new QToolButton(m_idle);
    m_play->setObjectName(QStringLiteral("cellPlay"));
    m_play->setText(QString());
    m_play->setIcon(makeCellIcon(QStringLiteral("play"), QColor(28, 31, 35)));
    m_play->setIconSize(QSize(28, 28));
    m_play->setFixedSize(66, 66);
    m_play->setToolTip(T("Запустить камеру"));
    m_logo = new QLabel(m_idle);
    m_logo->setObjectName(QStringLiteral("emptyCellBrand"));
    m_logo->setAlignment(Qt::AlignCenter);
    m_logo->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    // Полупрозрачная векторная эмблема: размер и положение автоматически
    // подстраиваются под фактические габариты каждой ячейки видеомозаики.
    m_logo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    auto *logoOpacity = new QGraphicsOpacityEffect(m_logo);
    logoOpacity->setOpacity(0.92);
    m_logo->setGraphicsEffect(logoOpacity);
    m_hint = new QLabel(m_idle);
    m_hint->setObjectName(QStringLiteral("placeholder"));
    m_hint->setAlignment(Qt::AlignCenter);
    m_hint->setWordWrap(true);
    m_hint->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    idleLay->addStretch(1);
    idleLay->addWidget(m_play, 0, Qt::AlignHCenter);
    idleLay->addWidget(m_logo, 0, Qt::AlignHCenter);
    idleLay->addWidget(m_hint);
    idleLay->addStretch(1);

    m_stack = new QStackedLayout;
    m_stack->addWidget(m_idle);
    m_stack->addWidget(m_surface);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(4, 4, 4, 4);
    root->setSpacing(1);
    root->addWidget(m_bar);
    root->addLayout(m_stack, 1);

    // ---- воспроизведение ----
    m_player = new QMediaPlayer(this);
    m_player->setVideoSink(m_surface->sink());

    m_retry = new QTimer(this);
    m_retry->setSingleShot(true);
    m_retry->setInterval(3000);
    connect(m_retry, &QTimer::timeout, this, [this] { if (m_started) restart(); });
    connect(m_player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error err) {
        if (!m_started)
            return;
        m_surface->setLinkQuality(VideoSurface::LinkQuality::None);
        // AccessDeniedError — камера ответила «неверный логин или пароль»: вместо бесконечных
        // повторов сразу спрашиваем пароль прямо в ячейке
        if (err == QMediaPlayer::AccessDeniedError && !isLocalCapture()) {
            m_retry->stop();
            m_surface->setStatusText(T("Нужен пароль"));
            showAuthPrompt();
        } else {
            m_surface->setStatusText(T("Нет видеопотока"));
            m_retry->start();
        }
    });
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus st) {
        if (!m_started)
            return;
        if (st == QMediaPlayer::LoadingMedia) {
            m_surface->setStatusText(T("Подключение…"));
            m_surface->setLinkQuality(VideoSurface::LinkQuality::Connecting);
        } else if (st == QMediaPlayer::EndOfMedia || st == QMediaPlayer::InvalidMedia) {
            m_surface->setStatusText(T("Нет видеопотока"));
            m_surface->setLinkQuality(VideoSurface::LinkQuality::None);
            m_retry->start();
        }
    });
    // QMediaPlayer может восстановить RTSP-соединение после краткого обрыва,
    // хотя новый кадр приходит чуть позже. Снимаем устаревшую надпись «Нет сигнала»
    // сразу после перехода плеера в PlayingState; onFrame() уберёт ожидание при
    // поступлении первого декодированного кадра.
    connect(m_player, &QMediaPlayer::playbackStateChanged, this,
            [this](QMediaPlayer::PlaybackState state) {
        if (!m_started)
            return;
        if (state == QMediaPlayer::PlayingState && m_surface->currentFrame().isNull()) {
            m_surface->setStatusText(T("Ожидание видеопотока…"));
            m_surface->setLinkQuality(VideoSurface::LinkQuality::Connecting);
        }
    });

    // ---- кнопки и сигналы поверхности ----
    connect(m_closeBtn, &QToolButton::clicked, this, [this] { clear(true); });
    connect(m_audioBtn, &QToolButton::clicked, this, [this] { setMuted(!m_muted); });
    connect(m_volumeSlider, &QSlider::valueChanged, this, [this](int value) {
        if (!m_audioOut) m_audioOut = new QAudioOutput(this);
        m_audioOut->setVolume(value / 100.0);
        if (value > 0 && m_muted) setMuted(false);
        if (value == 0 && !m_muted) m_audioOut->setMuted(true);
        else if (m_audioOut) m_audioOut->setMuted(false);
    });
    connect(m_play, &QToolButton::clicked, this, [this] { startPlayback(); });
    connect(m_surface, &VideoSurface::clicked, this, [this] { emit clicked(this); });
    connect(m_surface, &VideoSurface::doubleClicked, this, [this] { emit doubleClicked(this); });
    connect(m_surface, &VideoSurface::contextMenuRequested, this,
            [this](const QPoint &pos) { emit contextMenuRequested(this, pos); });
    m_idle->installEventFilter(this);
    // Разрешаем перетаскивание не только за заголовок, но и прямо за изображение камеры.
    m_surface->installEventFilter(this);

    setMuted(true);
    clear(false);
    fitHintFont();
    updateCellArtwork();
}

// ---------------------------------------------------------------- назначение камеры

VideoSurface *VideoCell::takeSurfaceForFullscreen()
{
    if (!m_surface)
        return nullptr;
    m_stack->removeWidget(m_surface);
    m_surface->setParent(nullptr);
    return m_surface;
}

void VideoCell::restoreSurfaceFromFullscreen(VideoSurface *surface)
{
    if (!surface || surface != m_surface)
        return;
    surface->setParent(this);
    m_stack->insertWidget(1, surface);
    m_stack->setCurrentWidget(m_started ? m_surface : m_idle);
}

void VideoCell::setCamera(const QString &id, const QString &title, const QUrl &url, bool autoplay,
                          bool showTime, int brightness, bool audio, bool fillCell)
{
    m_cameraId = id;
    m_url = url;
    setViewOptions(title, showTime, brightness);
    setMuted(!audio);
    setFillCell(fillCell);
    m_audioBtn->setVisible(true);
    m_closeBtn->setVisible(true);
    if (autoplay)
        startPlayback();
    else
        stopPlayback();
    emit assignmentChanged();
}

void VideoCell::setViewOptions(const QString &title, bool showTime, int brightness)
{
    m_titleText = title;
    m_title->setText(title);
    m_surface->setShowTime(showTime);
    m_surface->setBrightness(brightness);
    updateIdlePage();
}

void VideoCell::setFillCell(bool on)
{
    if (m_surface) m_surface->setFillCell(on);
}

void VideoCell::startPlayback()
{
    if (m_cameraId.isEmpty())
        return;
    m_started = true;
    m_surface->setCameraRunning(true);
    m_stack->setCurrentWidget(m_surface);
    m_surface->setStatusText(T("Подключение…"));
    m_surface->setLinkQuality(VideoSurface::LinkQuality::Connecting);
    restart();
}

void VideoCell::stopPlayback()
{
    m_started = false;
    m_surface->setCameraRunning(false);
    m_retry->stop();
    hideAuthPrompt();
    m_player->stop();
    m_player->setSource(QUrl());
    if (m_camera)
        m_camera->stop();
    m_surface->clearFrame();
    m_stack->setCurrentWidget(m_idle);
    updateIdlePage();
}

void VideoCell::clear(bool notify)
{
    m_started = false;
    m_surface->setCameraRunning(false);
    m_retry->stop();
    hideAuthPrompt();
    m_player->stop();
    m_player->setSource(QUrl());
    if (m_camera)
        m_camera->stop();
    m_surface->clearFrame();
    m_surface->setStatusText(QString());
    m_cameraId.clear();
    m_titleText.clear();
    m_url.clear();
    m_title->clear();
    setMuted(true);
    m_audioBtn->setVisible(true);
    m_closeBtn->setVisible(false);
    m_stack->setCurrentWidget(m_idle);
    updateIdlePage();
    if (notify)
        emit assignmentChanged();
}

bool VideoCell::isLocalCapture() const
{
    return m_url.scheme() == QLatin1String("qcam");
}

// ---------------------------------------------------------------- пароль прямо в ячейке

void VideoCell::showAuthPrompt()
{
    if (!m_authBox) {
        m_authBox = new QWidget(this);
        m_authBox->setObjectName(QStringLiteral("cellAuthBox"));
        auto *lay = new QVBoxLayout(m_authBox);
        lay->setContentsMargins(14, 12, 14, 12);
        lay->setSpacing(6);

        auto *title = new QLabel(T("Камера требует пароль"), m_authBox);
        title->setObjectName(QStringLiteral("cellAuthTitle"));
        m_authHost = new QLabel(m_authBox);
        m_authHost->setWordWrap(true);

        m_authUser = new QLineEdit(m_authBox);
        m_authUser->setPlaceholderText(T("Логин"));
        QWidget *passRow = passwordFieldWithEye(m_authBox, &m_authPass);
        m_authPass->setPlaceholderText(T("Пароль"));
        connect(m_authPass, &QLineEdit::returnPressed, this, &VideoCell::submitAuthPrompt);
        connect(m_authUser, &QLineEdit::returnPressed, this, [this] { m_authPass->setFocus(); });

        m_authBtn = new QPushButton(T("Подключить"), m_authBox);
        m_authBtn->setObjectName(QStringLiteral("cellAuthBtn"));
        connect(m_authBtn, &QPushButton::clicked, this, &VideoCell::submitAuthPrompt);

        lay->addWidget(title);
        lay->addWidget(m_authHost);
        lay->addWidget(m_authUser);
        lay->addWidget(passRow);
        lay->addWidget(m_authBtn);
        lay->addStretch(1);
    }

    m_authHost->setText(m_url.host() + (m_url.port() > 0 ? QStringLiteral(":%1").arg(m_url.port()) : QString()));
    if (m_authUser->text().isEmpty())
        m_authUser->setText(m_url.userName());
    m_authShown = true;
    m_authBox->setGeometry(0, m_bar->height(), width(), height() - m_bar->height());
    m_authBox->show();
    m_authBox->raise();
    m_authUser->text().isEmpty() ? m_authUser->setFocus() : m_authPass->setFocus();
}

void VideoCell::hideAuthPrompt()
{
    if (m_authBox)
        m_authBox->hide();
    m_authShown = false;
}

void VideoCell::submitAuthPrompt()
{
    if (!m_authBox)
        return;
    QUrl newUrl = m_url;
    newUrl.setUserName(m_authUser->text().trimmed());
    newUrl.setPassword(m_authPass->text());
    hideAuthPrompt();
    m_url = newUrl;
    emit credentialsEntered(this, newUrl);   // пусть владелец сохранит пароль в настройках камеры
    if (m_started)
        restart();
}

void VideoCell::restart()
{
    hideAuthPrompt();
    // При смене основного/дополнительного потока не оставляем на экране
    // последний кадр старого источника и сбрасываем ограничитель FPS.
    if (m_surface) {
        m_surface->clearFrame();
        m_surface->setStatusText(T("Подключение…"));
        m_surface->setLinkQuality(VideoSurface::LinkQuality::Connecting);
    }
    if (isLocalCapture()) {
        // Аналоговая камера через карту видеозахвата: обычный QCamera вместо RTSP-плеера
        m_player->stop();
        m_player->setSource(QUrl());
        if (!m_camera) {
            m_camera = new QCamera(this);
            m_capture = new QMediaCaptureSession(this);
            m_capture->setCamera(m_camera);
            m_capture->setVideoSink(m_surface->sink());
            connect(m_camera, &QCamera::activeChanged, this, [this](bool active) {
                if (!active && m_started) {
                    m_surface->setStatusText(T("Нет видеопотока"));
                    m_surface->setLinkQuality(VideoSurface::LinkQuality::None);
                }
            });
            connect(m_camera, &QCamera::errorOccurred, this, [this] {
                m_surface->setStatusText(T("Нет видеопотока"));
                m_surface->setLinkQuality(VideoSurface::LinkQuality::None);
            });
        }
        const QByteArray devId = QByteArray::fromBase64(m_url.path().toLatin1());
        for (const QCameraDevice &d : QMediaDevices::videoInputs()) {
            if (d.id() == devId) {
                m_camera->setCameraDevice(d);
                break;
            }
        }
        m_camera->start();
        return;
    }
    if (m_camera)
        m_camera->stop();
    m_player->stop();
    m_player->setSource(QUrl());
    m_player->setSource(m_url);
    m_player->play();
}

void VideoCell::setMuted(bool muted)
{
    m_muted = muted;
    if (muted) {
        m_player->setAudioOutput(nullptr);       // без звука аудиопоток не обрабатывается: меньше нагрузка
    } else {
        if (!m_audioOut)
            m_audioOut = new QAudioOutput(this);
        m_audioOut->setVolume(m_volumeSlider ? m_volumeSlider->value() / 100.0 : 0.70);
        m_audioOut->setMuted(false);
        m_player->setAudioOutput(m_audioOut);
    }
    if (m_volumeSlider) m_volumeSlider->setEnabled(true);
    m_audioBtn->setText(QString());
    m_audioBtn->setIcon(makeCellIcon(muted ? QStringLiteral("mute") : QStringLiteral("sound")));
}

QImage VideoCell::snapshot() const
{
    return m_surface->currentFrame();
}

void VideoCell::resetZoom()
{
    m_surface->resetZoom();
}

void VideoCell::setSelected(bool on)
{
    setProperty("selected", on);
    style()->unpolish(this);
    style()->polish(this);
    update();
}

void VideoCell::setDropTarget(bool on)
{
    setProperty("dropTarget", on);
    style()->unpolish(this);
    style()->polish(this);
    update();
}

// ---------------------------------------------------------------- «пустая» страница

void VideoCell::updateIdlePage()
{
    const bool empty = m_cameraId.isEmpty();
    m_play->setVisible(!empty && !m_started);
    if (m_logo)
        m_logo->setVisible(!m_started);
    updateCellArtwork();
    if (empty)
        m_hint->setText(T("Перетащите камеру сюда"));
    else
        m_hint->setText(m_titleText + QStringLiteral("\n") + T("Нажмите ▶, чтобы запустить"));
}

void VideoCell::updateCellArtwork()
{
    if (!m_logo || !m_idle)
        return;
    const int available = qMax(1, qMin(m_idle->width() - 28, m_idle->height() / 2));
    const int side = qBound(46, qMin(136, available), 136);
    if (m_logo->size() != QSize(side, side))
        m_logo->setFixedSize(side, side);
    m_logo->setPixmap(makeCameraCellArtwork(QSize(side, side)));
    m_logo->setAlignment(Qt::AlignCenter);
}

void VideoCell::enterEvent(QEnterEvent *e)
{
    m_hover = true;
    updateIdlePage();
    QFrame::enterEvent(e);
}

void VideoCell::leaveEvent(QEvent *e)
{
    m_hover = false;
    updateIdlePage();
    QFrame::leaveEvent(e);
}

// ---------------------------------------------------------------- вспышка после перестановки

void VideoCell::flash()
{
    if (!m_flash) {
        m_flash = new QLabel(this);
        m_flash->setObjectName(QStringLiteral("cellFlash"));
        m_flash->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_flashFx = new QGraphicsOpacityEffect(m_flash);
        m_flash->setGraphicsEffect(m_flashFx);
    }
    m_flash->setGeometry(rect());
    m_flash->show();
    m_flash->raise();
    if (!Cfg::view().animations) {
        QTimer::singleShot(150, m_flash, &QWidget::hide);
        return;
    }
    auto *a = new QPropertyAnimation(m_flashFx, "opacity", this);
    a->setDuration(620);
    a->setKeyValueAt(0.0, 0.0);
    a->setKeyValueAt(0.16, 0.92);
    a->setKeyValueAt(0.42, 0.18);
    a->setKeyValueAt(0.62, 0.58);
    a->setKeyValueAt(1.0, 0.0);
    a->setEasingCurve(QEasingCurve::OutCubic);
    connect(a, &QPropertyAnimation::finished, m_flash, &QWidget::hide);
    a->start(QAbstractAnimation::DeleteWhenStopped);
}

void VideoCell::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);
    if (m_flash && m_flash->isVisible())
        m_flash->setGeometry(rect());
    if (m_authBox && m_authShown)
        m_authBox->setGeometry(0, m_bar->height(), width(), height() - m_bar->height());
    fitHintFont();
    updateCellArtwork();
}

// Подпись «Перетащите камеру сюда» подстраивается под размер ячейки: мелкая ячейка — мелкий шрифт,
// крупная — крупнее, в разумных пределах.
void VideoCell::fitHintFont()
{
    const int size = qBound(10, width() / 16, 20);
    QFont f = m_hint->font();
    if (f.pixelSize() != size) {
        f.setPixelSize(size);
        m_hint->setFont(f);
    }
}

// ---------------------------------------------------------------- перетаскивание

QPixmap VideoCell::dragPixmap() const
{
    QPixmap pm(260, 84);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setOpacity(0.92);
    p.setBrush(QColor(0x12, 0x13, 0x16, 210));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(QRectF(0, 3, 260, 80), 14, 14);
    QLinearGradient shell(0, 2, 0, 82);
    shell.setColorAt(0.0, QColor(0x4a, 0x45, 0x35));
    shell.setColorAt(0.18, QColor(0x25, 0x25, 0x28));
    shell.setColorAt(1.0, QColor(0x12, 0x13, 0x16));
    p.setBrush(shell);
    p.setPen(QPen(QColor(0xf2, 0xc2, 0x00), 2));
    p.drawRoundedRect(QRectF(2, 2, 256, 78), 12, 12);
    p.setPen(QPen(QColor(0xff, 0xe0, 0x72, 150), 1));
    p.drawRoundedRect(QRectF(5, 5, 250, 72), 9, 9);

    const QImage shot = snapshot();
    int textX = 16;
    if (!shot.isNull()) {
        const QImage thumb = shot.scaled(QSize(96, 54), Qt::KeepAspectRatio, Qt::FastTransformation);
        p.drawImage(QPoint(14, 15 + (54 - thumb.height()) / 2), thumb);
        textX = 122;
    }
    QFont f = p.font();
    f.setBold(true);
    f.setPixelSize(13);
    p.setFont(f);
    p.setPen(QColor(0xf2, 0xc2, 0x00));
    const QString name = QFontMetrics(f).elidedText(m_titleText, Qt::ElideRight, 250 - textX);
    p.drawText(QRect(textX, 16, 250 - textX, 24), Qt::AlignVCenter | Qt::AlignLeft, name);
    f.setBold(false);
    f.setPixelSize(11);
    p.setFont(f);
    p.setPen(QColor(0xb4, 0xb4, 0xb8));
    p.drawText(QRect(textX, 42, 250 - textX, 30), Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap,
               T("Перетащите на другую ячейку"));
    return pm;
}

void VideoCell::startDrag()
{
    auto *drag = new QDrag(this);
    auto *mime = new QMimeData;
    mime->setData(QString::fromLatin1(kCellMime), QByteArray::number(qulonglong(reinterpret_cast<quintptr>(this))));
    drag->setMimeData(mime);
    const QPixmap pm = dragPixmap();
    drag->setPixmap(pm);
    drag->setHotSpot(QPoint(pm.width() / 2, pm.height() / 2));
    setProperty("dragSource", true);
    style()->unpolish(this);
    style()->polish(this);
    drag->exec(Qt::MoveAction);
    setProperty("dragSource", false);
    style()->unpolish(this);
    style()->polish(this);
}

bool VideoCell::eventFilter(QObject *obj, QEvent *e)
{
    // Заголовок и само видео — полноценные зоны захвата. На видео не перехватываем
    // обычный клик/двойной клик: ими по-прежнему управляет VideoSurface.
    if (obj == m_title) {
        if (e->type() == QEvent::MouseButtonPress) {
            auto *me = static_cast<QMouseEvent *>(e);
            if (me->button() == Qt::LeftButton) {
                m_dragStart = me->pos();
                emit clicked(this);
            }
        } else if (e->type() == QEvent::MouseMove) {
            auto *me = static_cast<QMouseEvent *>(e);
            if ((me->buttons() & Qt::LeftButton)
                && (me->pos() - m_dragStart).manhattanLength() > QApplication::startDragDistance()) {
                startDrag();
                return true;
            }
        } else if (e->type() == QEvent::MouseButtonDblClick) {
            emit doubleClicked(this);
            return true;
        }
    } else if (obj == m_surface) {
        if (e->type() == QEvent::MouseButtonPress) {
            auto *me = static_cast<QMouseEvent *>(e);
            if (me->button() == Qt::LeftButton)
                m_dragStart = me->pos();
        } else if (e->type() == QEvent::MouseMove) {
            auto *me = static_cast<QMouseEvent *>(e);
            if ((me->buttons() & Qt::LeftButton)
                && (me->pos() - m_dragStart).manhattanLength() > QApplication::startDragDistance()) {
                startDrag();
                return true;
            }
        }
    } else if (obj == m_idle) {
        if (e->type() == QEvent::MouseButtonPress) {
            auto *me = static_cast<QMouseEvent *>(e);
            if (me->button() == Qt::LeftButton) {
                m_dragStart = me->pos();
                emit clicked(this);
            }
        } else if (e->type() == QEvent::MouseMove) {
            auto *me = static_cast<QMouseEvent *>(e);
            if ((me->buttons() & Qt::LeftButton)
                && (me->pos() - m_dragStart).manhattanLength() > QApplication::startDragDistance()) {
                startDrag();
                return true;
            }
        }
    }
    return QFrame::eventFilter(obj, e);
}

void VideoCell::mousePressEvent(QMouseEvent *e)
{
    emit clicked(this);
    QFrame::mousePressEvent(e);
}

void VideoCell::contextMenuEvent(QContextMenuEvent *e)
{
    emit contextMenuRequested(this, e->globalPos());
    e->accept();
}

void VideoCell::dragEnterEvent(QDragEnterEvent *e)
{
    const QMimeData *m = e->mimeData();
    if (m->hasFormat(QString::fromLatin1(kCameraMime)) || m->hasFormat(QString::fromLatin1(kCellMime))) {
        e->acceptProposedAction();
        setDropTarget(true);
    }
}

void VideoCell::dragLeaveEvent(QDragLeaveEvent *e)
{
    setDropTarget(false);
    QFrame::dragLeaveEvent(e);
}

void VideoCell::dropEvent(QDropEvent *e)
{
    setDropTarget(false);
    const QMimeData *m = e->mimeData();
    if (m->hasFormat(QString::fromLatin1(kCellMime))) {
        const quintptr src = quintptr(m->data(QString::fromLatin1(kCellMime)).toULongLong());
        e->acceptProposedAction();
        if (src != reinterpret_cast<quintptr>(this))
            emit cellDropped(this, src);
    } else if (m->hasFormat(QString::fromLatin1(kCameraMime))) {
        e->acceptProposedAction();
        emit cameraDropped(this, QString::fromUtf8(m->data(QString::fromLatin1(kCameraMime))));
    }
}

// ======================================================================
// app/video_grid.cpp
// ======================================================================
namespace {

// Переносит логин и пароль из одного адреса в другой (тот же хост, другой путь/канал) —
// используется, когда пароль ввели для одного потока камеры, а применить нужно и ко второму.
QString withSameCredentials(const QString &target, const QUrl &credsFrom)
{
    QUrl u(target);
    if (!u.isValid())
        return target;
    u.setUserName(credsFrom.userName());
    u.setPassword(credsFrom.password());
    return u.toString(QUrl::FullyEncoded);
}

struct GridTemplate {
    QString name;
    int count = 4;
    QStringList cells;
};

QList<GridTemplate> readGridTemplates()
{
    QList<GridTemplate> out;
    QSettings s;
    const int n = s.beginReadArray(QStringLiteral("templates"));
    for (int i = 0; i < n; ++i) {
        s.setArrayIndex(i);
        GridTemplate t;
        t.name = s.value(QStringLiteral("name")).toString();
        t.count = s.value(QStringLiteral("count"), 4).toInt();
        t.cells = s.value(QStringLiteral("cells")).toStringList();
        if (!t.name.isEmpty())
            out.append(t);
    }
    s.endArray();
    return out;
}

void writeGridTemplates(const QList<GridTemplate> &v)
{
    QSettings s;
    s.remove(QStringLiteral("templates"));
    s.beginWriteArray(QStringLiteral("templates"), int(v.size()));
    for (int i = 0; i < v.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue(QStringLiteral("name"), v.at(i).name);
        s.setValue(QStringLiteral("count"), v.at(i).count);
        s.setValue(QStringLiteral("cells"), v.at(i).cells);
    }
    s.endArray();
}

int gridColumnsFor(int count)
{
    for (const GridLayoutDef &d : VideoGrid::layouts())
        if (d.count == count)
            return d.cols;
    return 2;
}

template <typename F>
QAction *addGridAction(QMenu *menu, const QString &text, QObject *ctx, F fn)
{
    QAction *a = menu->addAction(text);
    QObject::connect(a, &QAction::triggered, ctx, fn);
    return a;
}

} // namespace

QList<GridLayoutDef> VideoGrid::layouts()
{
    // 6, 8, 12 и 20 ячеек удобны для широкоформатных экранов 16:9 и 21:9
    return {{1, 1}, {4, 2}, {6, 3}, {8, 4}, {9, 3}, {12, 4}, {16, 4}, {20, 5}, {25, 5}, {36, 6}, {49, 7}, {64, 8}};
}

QString VideoGrid::layoutLabel(int count)
{
    const int cols = gridColumnsFor(count);
    const int rows = (count + cols - 1) / cols;
    return QStringLiteral("%1 × %2").arg(cols).arg(rows);
}

VideoGrid::VideoGrid(CameraManager *mgr, QWidget *parent) : QWidget(parent), m_mgr(mgr)
{
    m_grid = new QGridLayout(this);
    m_grid->setContentsMargins(0, 0, 0, 0);
    m_grid->setSpacing(3);

    m_saveTimer = new QTimer(this);
    m_saveTimer->setSingleShot(true);
    m_saveTimer->setInterval(400);
    connect(m_saveTimer, &QTimer::timeout, this, [this] { saveLayoutNow(); });

    ensureCells(m_count);
    applyLayout();
    select(m_cells.first());

    connect(m_mgr, &CameraManager::camerasChanged, this, &VideoGrid::refreshCells);
}

void VideoGrid::ensureCells(int n)
{
    while (m_cells.size() < n) {
        auto *cell = new VideoCell(this);
        connect(cell, &VideoCell::clicked, this, &VideoGrid::select);
        connect(cell, &VideoCell::doubleClicked, this, &VideoGrid::openFullscreen);
        connect(cell, &VideoCell::cameraDropped, this, [this](VideoCell *c, const QString &id) {
            placeCamera(c, id, true);
            select(c);
            c->flash();
        });
        connect(cell, &VideoCell::cellDropped, this, [this](VideoCell *c, quintptr src) {
            for (VideoCell *o : std::as_const(m_cells)) {
                if (reinterpret_cast<quintptr>(o) == src) {
                    swapCells(o, c);
                    return;
                }
            }
        });
        connect(cell, &VideoCell::contextMenuRequested, this, &VideoGrid::showCellMenu);
        connect(cell, &VideoCell::addRequested, this, [this](VideoCell *c) {
            select(c);
            showAddMenu(c, QCursor::pos());
        });
        connect(cell, &VideoCell::assignmentChanged, this, &VideoGrid::saveSoon);
        connect(cell, &VideoCell::credentialsEntered, this, [this](VideoCell *c, const QUrl &newUrl) {
            const CameraEntry *e = m_mgr->find(c->cameraId());
            if (!e)
                return;
            CameraEntry upd = *e;
            // Логин и пароль относятся к самой камере, поэтому подставляем их в оба адреса —
            // основной поток (запись) и подпоток (мозаика), если он задан
            upd.url = withSameCredentials(e->url, newUrl);
            if (!e->liveUrl.isEmpty())
                upd.liveUrl = withSameCredentials(e->liveUrl, newUrl);
            m_mgr->update(upd);
            emit message(T("Пароль камеры «%1» сохранён").arg(e->name));
        });
        m_cells.append(cell);
    }
}

void VideoGrid::applyLayout()
{
    for (VideoCell *c : std::as_const(m_cells))
        m_grid->removeWidget(c);
    for (int i = 0; i < 10; ++i) {
        m_grid->setRowStretch(i, 0);
        m_grid->setColumnStretch(i, 0);
    }
    const int cols = gridColumnsFor(m_count);
    const int rows = (m_count + cols - 1) / cols;
    for (int i = 0; i < m_cells.size(); ++i) {
        VideoCell *c = m_cells.at(i);
        if (i < m_count) {
            m_grid->addWidget(c, i / cols, i % cols);
            c->show();
        } else {
            c->hide();
        }
    }
    for (int i = 0; i < cols; ++i)
        m_grid->setColumnStretch(i, 1);
    for (int i = 0; i < rows; ++i)
        m_grid->setRowStretch(i, 1);
}

void VideoGrid::setLayoutCount(int n)
{
    bool valid = false;
    for (const GridLayoutDef &d : layouts())
        if (d.count == n)
            valid = true;
    if (!valid)
        return;

    m_count = n;
    ensureCells(n);
    for (int i = n; i < m_cells.size(); ++i)    // скрытые ячейки не должны качать видео
        m_cells.at(i)->clear(false);
    applyLayout();
    if (m_selected && m_cells.indexOf(m_selected) >= n)
        select(m_cells.first());
    refreshCells();                              // в раскладке 1 × 1 камера переключается на основной поток
    emit layoutCountChanged(n);
    saveSoon();
}

void VideoGrid::select(VideoCell *cell)
{
    if (m_selected == cell)
        return;
    if (m_selected)
        m_selected->setSelected(false);
    m_selected = cell;
    if (m_selected)
        m_selected->setSelected(true);
    emit selectionChanged(cell ? cell->cameraId() : QString());
}

QStringList VideoGrid::cameraIds() const
{
    QStringList ids;
    for (int i = 0; i < m_count && i < m_cells.size(); ++i)
        ids << m_cells.at(i)->cameraId();
    return ids;
}

// Основной поток или подпоток — по настройке качества камеры; «авто»: подпоток в мозаике, основной в 1 × 1
QUrl VideoGrid::urlFor(const CameraEntry &e, const VideoCell *) const
{
    bool wantMain;
    if (e.viewQuality == 1)
        wantMain = true;
    else if (e.viewQuality == 2)
        wantMain = false;
    else
        wantMain = (m_count == 1);
    return QUrl((wantMain || e.liveUrl.isEmpty()) ? e.url : e.liveUrl);
}

void VideoGrid::placeCamera(VideoCell *cell, const QString &cameraId, bool autoplay)
{
    const CameraEntry *e = m_mgr->find(cameraId);
    if (!e)
        return;
    cell->setCamera(e->id, e->name, urlFor(*e, cell), autoplay, e->showTime, e->brightness, e->audio, e->fillCell);
}

void VideoGrid::openCamera(const QString &cameraId)
{
    VideoCell *target = nullptr;
    if (m_selected && m_selected->isVisible() && m_selected->isEmpty())
        target = m_selected;
    if (!target) {
        for (VideoCell *c : std::as_const(m_cells)) {
            if (c->isVisible() && c->isEmpty()) {
                target = c;
                break;
            }
        }
    }
    if (!target)
        target = (m_selected && m_selected->isVisible()) ? m_selected : m_cells.first();

    placeCamera(target, cameraId, true);

    // выделяем следующую свободную ячейку, чтобы можно было открывать камеры подряд
    VideoCell *next = target;
    for (VideoCell *c : std::as_const(m_cells)) {
        if (c->isVisible() && c->isEmpty()) {
            next = c;
            break;
        }
    }
    select(next);
}

void VideoGrid::clearAll()
{
    for (VideoCell *c : std::as_const(m_cells))
        c->clear(false);
    saveSoon();
}

void VideoGrid::startAllCells()
{
    for (int i = 0; i < m_count && i < m_cells.size(); ++i)
        if (!m_cells.at(i)->isEmpty())
            m_cells.at(i)->startPlayback();
}

void VideoGrid::stopAllCells()
{
    for (int i = 0; i < m_count && i < m_cells.size(); ++i)
        if (!m_cells.at(i)->isEmpty())
            m_cells.at(i)->stopPlayback();
}

void VideoGrid::refreshCells()
{
    for (VideoCell *c : std::as_const(m_cells)) {
        if (c->isEmpty())
            continue;
        const CameraEntry *e = m_mgr->find(c->cameraId());
        if (!e) {
            c->clear(false);                       // камера удалена
            saveSoon();
        } else if (urlFor(*e, c) != c->url()) {
            placeCamera(c, e->id, c->isStarted());  // сменился адрес или нужный поток
        } else {
            c->setViewOptions(e->name, e->showTime, e->brightness);
            c->setFillCell(e->fillCell);
        }
    }
}

// Обмен камер местами: ячейки подсвечиваются вспышкой
void VideoGrid::swapCells(VideoCell *a, VideoCell *b)
{
    const QString ia = a->cameraId(), ib = b->cameraId();
    const bool pa = a->isStarted(), pb = b->isStarted();
    a->clear(false);
    b->clear(false);
    if (!ib.isEmpty())
        placeCamera(a, ib, pb);
    if (!ia.isEmpty())
        placeCamera(b, ia, pa);
    a->flash();
    b->flash();
    select(b);
    // Перестановка фиксируется на диске сразу: перезапуск потока или приложения
    // не должен менять назначение камер ячейкам.
    saveLayoutNow();
}

// ---------------------------------------------------------------- шаблоны

void VideoGrid::saveSoon()
{
    if (!m_restoring)
        m_saveTimer->start();
}

void VideoGrid::saveLayoutNow()
{
    QSettings s;
    s.setValue(QStringLiteral("layout/count"), m_count);
    s.setValue(QStringLiteral("layout/cells"), cameraIds());
    s.sync(); // записать перестановку на диск сразу после debounce-сохранения
}

void VideoGrid::applyTemplateData(int count, const QStringList &ids, bool autoplay)
{
    m_restoring = true;                              // промежуточные состояния не сохраняем
    for (VideoCell *c : std::as_const(m_cells))
        c->clear(false);
    bool valid = false;
    for (const GridLayoutDef &d : layouts())
        if (d.count == count)
            valid = true;
    setLayoutCount(valid ? count : 4);
    for (int i = 0; i < ids.size() && i < m_count; ++i)
        if (!ids.at(i).isEmpty() && m_mgr->find(ids.at(i)))
            placeCamera(m_cells.at(i), ids.at(i), autoplay);
    m_restoring = false;
    saveSoon();
}

void VideoGrid::restoreLayout()
{
    QSettings s;
    const int count = s.value(QStringLiteral("layout/count"), 4).toInt();
    const QStringList ids = s.value(QStringLiteral("layout/cells")).toStringList();
    // при отключённом автозапуске камеры занимают свои ячейки, но не стартуют (кнопка ▶)
    applyTemplateData(count, ids, Cfg::autostartCameras());
}

QStringList VideoGrid::templateNames() const
{
    QStringList names;
    for (const GridTemplate &t : readGridTemplates())
        names << t.name;
    return names;
}

void VideoGrid::saveTemplate(const QString &name)
{
    QList<GridTemplate> all = readGridTemplates();
    GridTemplate t;
    t.name = name;
    t.count = m_count;
    t.cells = cameraIds();
    bool replaced = false;
    for (GridTemplate &x : all) {
        if (x.name == name) {
            x = t;
            replaced = true;
        }
    }
    if (!replaced)
        all.append(t);
    writeGridTemplates(all);
    emit message(T("Шаблон «%1» сохранён").arg(name));
}

void VideoGrid::applyTemplateNamed(const QString &name)
{
    for (const GridTemplate &t : readGridTemplates()) {
        if (t.name == name) {
            applyTemplateData(t.count, t.cells, true);
            emit message(T("Применён шаблон «%1»").arg(name));
            return;
        }
    }
}

void VideoGrid::removeTemplate(const QString &name)
{
    QList<GridTemplate> all = readGridTemplates();
    for (int i = 0; i < all.size(); ++i) {
        if (all.at(i).name == name) {
            all.removeAt(i);
            break;
        }
    }
    writeGridTemplates(all);
}

// ---------------------------------------------------------------- меню

void VideoGrid::showAddMenu(VideoCell *cell, const QPoint &globalPos)
{
    QMenu menu(this);
    const QStringList shown = cameraIds();
    QMap<QString, QList<const CameraEntry *>> byGroup;
    for (const CameraEntry &e : m_mgr->cameras())
        byGroup[e.group].append(&e);

    QPointer<VideoCell> pc(cell);
    auto addCam = [&](QMenu *m, const CameraEntry *e) {
        const QString id = e->id;
        const QString text = (shown.contains(id) ? QStringLiteral("✓ ") : QString()) + e->name;
        addGridAction(m, text, this, [this, id, pc] {
            if (!pc)
                return;
            placeCamera(pc, id, true);
            select(pc);
            pc->flash();
        });
    };
    for (auto it = byGroup.begin(); it != byGroup.end(); ++it) {
        if (it.key().isEmpty())
            continue;
        QMenu *sub = menu.addMenu(it.key());
        for (const CameraEntry *e : std::as_const(it.value()))
            addCam(sub, e);
    }
    for (const CameraEntry *e : byGroup.value(QString()))
        addCam(&menu, e);
    if (m_mgr->cameras().isEmpty())
        menu.addAction(T("Камер пока нет"))->setEnabled(false);

    menu.addSeparator();
    addGridAction(&menu, T("Добавить новую камеру…"), this, [this] { emit addCameraRequested(); });
    menu.exec(globalPos);
}

void VideoGrid::showCellMenu(VideoCell *cell, const QPoint &globalPos)
{
    if (cell->isEmpty()) {
        select(cell);
        showAddMenu(cell, globalPos);
        return;
    }
    const CameraEntry *found = m_mgr->find(cell->cameraId());
    if (!found)
        return;
    select(cell);
    const CameraEntry entry = *found;             // копия: список камер может измениться, пока открыто меню
    const QString id = entry.id;
    QPointer<VideoCell> pc(cell);

    QMenu menu(this);
    menu.addAction(entry.name)->setEnabled(false);
    menu.addSeparator();

    addGridAction(&menu, T("Настройки камеры…"), this, [this, id] { editCamera(id); });

    // качество картинки
    QMenu *quality = menu.addMenu(T("Качество картинки"));
    auto *qGroup = new QActionGroup(quality);
    const QString qNames[3] = {T("Авто (подпоток в мозаике)"), T("Высокое (основной поток)"), T("Низкое (подпоток, экономия)")};
    for (int q = 0; q < 3; ++q) {
        QAction *a = addGridAction(quality, qNames[q], this, [this, id, q] {
            if (const CameraEntry *e = m_mgr->find(id)) {
                CameraEntry c = *e;
                c.viewQuality = q;
                m_mgr->update(c);
            }
        });
        a->setCheckable(true);
        a->setChecked(entry.viewQuality == q);
        qGroup->addAction(a);
    }

    // поток для мозаики
    const QList<StreamOption> options = streamOptionsFor(entry.url);
    QMenu *streams = menu.addMenu(T("Поток для мозаики"));
    auto *sGroup = new QActionGroup(streams);
    auto addStream = [&](const QString &text, const QString &url) {
        QAction *a = addGridAction(streams, text, this, [this, id, url] { m_mgr->setLiveUrl(id, url); });
        a->setCheckable(true);
        a->setChecked(url == entry.liveUrl);
        sGroup->addAction(a);
    };
    addStream(T("Как у записи (без подпотока)"), QString());
    bool known = entry.liveUrl.isEmpty();
    for (const StreamOption &o : options) {
        addStream(o.label, o.url);
        if (o.url == entry.liveUrl)
            known = true;
    }
    if (!known)
        addStream(T("Текущий подпоток"), entry.liveUrl);

    // Масштабирование: режим заполнения не зависит от выбранного видеопотока.
    QAction *fill = addGridAction(&menu, T("Заполнить всю ячейку (обрезать края)"), this, [this, id] {
        if (const CameraEntry *e = m_mgr->find(id)) {
            CameraEntry c = *e;
            c.fillCell = !c.fillCell;
            m_mgr->update(c);
        }
    });
    fill->setCheckable(true);
    fill->setChecked(entry.fillCell);
    addGridAction(&menu, T("Применить заполнение ко всем камерам"), this, [this] {
        for (const CameraEntry &e : m_mgr->cameras()) {
            CameraEntry c = e;
            c.fillCell = true;
            m_mgr->update(c);
        }
    });
    addGridAction(&menu, T("Отключить заполнение у всех камер"), this, [this] {
        for (const CameraEntry &e : m_mgr->cameras()) {
            CameraEntry c = e;
            c.fillCell = false;
            m_mgr->update(c);
        }
    });

    // звук и время
    QAction *sound = addGridAction(&menu, T("Звук"), this, [pc] { if (pc) pc->setMuted(!pc->isMuted()); });
    sound->setCheckable(true);
    sound->setChecked(!cell->isMuted());
    QAction *time = addGridAction(&menu, T("Дата и время на картинке"), this, [this, id] {
        if (const CameraEntry *e = m_mgr->find(id)) {
            CameraEntry c = *e;
            c.showTime = !c.showTime;
            m_mgr->update(c);
        }
    });
    time->setCheckable(true);
    time->setChecked(entry.showTime);

    menu.addSeparator();
    addGridAction(&menu, T("Снимок экрана"), this, [this, pc] { if (pc) saveSnapshot(pc); });
    addGridAction(&menu, T("На весь экран"), this, [this, pc] { if (pc) openFullscreen(pc); });
    addGridAction(&menu, T("Сбросить масштаб"), this, [pc] { if (pc) pc->resetZoom(); });

    menu.addSeparator();
    addGridAction(&menu, T("Архив камеры…"), this, [this, id] { emit archiveRequested(id); });
    if (m_mgr->isStarted(id))
        addGridAction(&menu, T("Остановить запись"), this, [this, id] { m_mgr->stopCamera(id); });
    else
        addGridAction(&menu, T("Запустить запись"), this, [this, id] { m_mgr->startCamera(id); });
    if (cell->isStarted())
        addGridAction(&menu, T("Остановить показ"), this, [pc] { if (pc) pc->stopPlayback(); });
    else
        addGridAction(&menu, T("Запустить показ"), this, [pc] { if (pc) pc->startPlayback(); });

    menu.addSeparator();
    addGridAction(&menu, T("Закрыть камеру"), this, [pc] { if (pc) pc->clear(true); });

    menu.exec(globalPos);
}

void VideoGrid::editCamera(const QString &cameraId)
{
    const CameraEntry *e = m_mgr->find(cameraId);
    if (!e)
        return;
    QStringList groups;
    for (const CameraEntry &c : m_mgr->cameras())
        if (!c.group.isEmpty() && !groups.contains(c.group))
            groups << c.group;
    CameraDialog dlg(*e, groups, window());
    if (dlg.exec() == QDialog::Accepted)
        m_mgr->update(dlg.entry());
}

void VideoGrid::saveSnapshot(VideoCell *cell)
{
    const QImage img = cell->snapshot();
    if (img.isNull()) {
        emit message(T("Нет кадра для снимка"));
        return;
    }
    const CameraEntry *e = m_mgr->find(cell->cameraId());
    QString safe = e ? e->name : QStringLiteral("camera");
    safe.replace(QRegularExpression(QStringLiteral("[^\\w\\- .]")), QStringLiteral("_"));

    QDir dir(Cfg::screenshotDir());
    dir.mkpath(QStringLiteral("."));
    const QString file = dir.filePath(QStringLiteral("%1_%2.png")
                                          .arg(safe, QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"))));
    if (img.save(file, "PNG"))
        emit message(T("Снимок сохранён: %1").arg(QDir::toNativeSeparators(file)));
    else
        emit message(T("Не удалось сохранить снимок: %1").arg(QDir::toNativeSeparators(file)));
}

// Двойной щелчок: камера на весь экран (основной поток)
void VideoGrid::openFullscreen(VideoCell *cell)
{
    if (cell->isEmpty())
        return;
    const CameraEntry *e = m_mgr->find(cell->cameraId());
    if (!e)
        return;
    auto *fs = new FullscreenView(cell);
    fs->setWindowTitle(e->name);
    fs->openOnScreenOf(window());
}

void VideoGrid::closeSelectedCell()
{
    if (m_selected && !m_selected->isEmpty())
        m_selected->clear(true);
}

QString VideoGrid::selectedCameraId() const
{
    return m_selected ? m_selected->cameraId() : QString();
}

void VideoGrid::flushLayout()
{
    if (m_saveTimer->isActive())
        m_saveTimer->stop();
    saveLayoutNow();
}

// ======================================================================
// app/video_surface.cpp
// ======================================================================
CameraHoverPreview::CameraHoverPreview(QTableWidget *table, CameraManager *manager, QObject *parent)
    : QObject(parent), m_table(table), m_manager(manager)
{
    if (!table) return;
    m_delay = new QTimer(this);
    m_delay->setSingleShot(true);
    m_delay->setInterval(350);
    connect(m_delay, &QTimer::timeout, this, &CameraHoverPreview::showPreview);

    m_popup = new QWidget(nullptr, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    m_popup->setAttribute(Qt::WA_ShowWithoutActivating);
    m_popup->setAttribute(Qt::WA_DeleteOnClose, false);
    m_popup->setObjectName(QStringLiteral("cameraHoverPreview"));
    m_popup->setStyleSheet(QStringLiteral("#cameraHoverPreview { background:#171b20; border:1px solid #c6a52b; border-radius:7px; }"));
    auto *layout = new QVBoxLayout(m_popup);
    layout->setContentsMargins(7, 7, 7, 7);
    layout->setSpacing(6);
    m_video = new VideoSurface(m_popup);
    m_video->setMinimumSize(320, 180);
    m_video->setMaximumSize(400, 225);
    m_video->setStatusText(T("Предпросмотр камеры"));
    m_info = new QLabel(m_popup);
    m_info->setWordWrap(true);
    m_info->setStyleSheet(QStringLiteral("color:#e7e9ec; padding:3px 5px;"));
    layout->addWidget(m_video);
    layout->addWidget(m_info);

    m_player = new QMediaPlayer(m_popup);
    m_audio = new QAudioOutput(m_popup);
    m_audio->setMuted(true);
    m_player->setAudioOutput(m_audio);
    m_player->setVideoOutput(m_video->sink());
    connect(m_player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error, const QString &error) {
        if (m_video && !error.isEmpty() && m_currentId.size())
            m_video->setStatusText(T("Нет изображения: %1").arg(error));
    });

    table->viewport()->installEventFilter(this);
}

CameraHoverPreview::~CameraHoverPreview()
{
    if (m_player) m_player->stop();
    delete m_popup;
    m_popup = nullptr;
}

bool CameraHoverPreview::eventFilter(QObject *watched, QEvent *event)
{
    if (!m_table || watched != m_table->viewport())
        return QObject::eventFilter(watched, event);
    if (event->type() == QEvent::MouseMove) {
        auto *mouse = static_cast<QMouseEvent *>(event);
        const QModelIndex index = m_table->indexAt(mouse->pos());
        if (index.isValid() && index.column() == 0) {
            const QTableWidgetItem *item = m_table->item(index.row(), 0);
            const QString id = item ? item->data(Qt::UserRole).toString() : QString();
            if (!id.isEmpty())
                schedule(id, m_table->viewport()->mapToGlobal(mouse->pos()));
            else
                hidePreview();
        } else {
            hidePreview();
        }
    } else if (event->type() == QEvent::Leave) {
        hidePreview();
    } else if (event->type() == QEvent::Hide) {
        hidePreview();
    }
    return QObject::eventFilter(watched, event);
}

void CameraHoverPreview::schedule(const QString &cameraId, const QPoint &globalPos)
{
    m_globalPos = globalPos;
    if (cameraId == m_currentId && m_popup->isVisible())
        return;
    if (cameraId != m_pendingId) {
        m_pendingId = cameraId;
        m_delay->start();
    }
}

void CameraHoverPreview::showPreview()
{
    if (!m_manager || !m_popup || m_pendingId.isEmpty()) return;
    const CameraEntry *entry = m_manager->find(m_pendingId);
    if (!entry) return;
    m_currentId = entry->id;
    const QString stream = entry->liveUrl.trimmed().isEmpty() ? entry->url : entry->liveUrl;
    m_player->stop();
    m_video->clearFrame();
    m_video->setStatusText(T("Подключение к камере…"));
    m_player->setSource(QUrl(stream));
    m_player->play();

    const QString link = m_manager->isOnline(entry->id) ? T("В сети") : T("Не в сети");
    const QString record = m_manager->isStarted(entry->id) ? stateText(m_manager->stateOf(entry->id)) : T("Не запущена");
    const QString safeUrl = QUrl(entry->url).toString(QUrl::RemoveUserInfo);
    m_info->setText(QStringLiteral("<b>%1</b><br>%2<br>%3: %4<br>%5: %6")
        .arg(entry->name.toHtmlEscaped(), entry->group.toHtmlEscaped(), T("Адрес"), safeUrl.toHtmlEscaped(),
             T("Связь / запись"), (link + QStringLiteral(" / ") + record).toHtmlEscaped()));
    QPoint pos = m_globalPos + QPoint(18, 18);
    m_popup->adjustSize();
    m_popup->move(pos);
    m_popup->show();
}

void CameraHoverPreview::hidePreview()
{
    if (m_delay) m_delay->stop();
    m_pendingId.clear();
    m_currentId.clear();
    if (m_player) m_player->stop();
    if (m_video) m_video->clearFrame();
    if (m_popup) m_popup->hide();
}

VideoSurface::VideoSurface(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMinimumSize(40, 30);

    m_sink = new QVideoSink(this);
    connect(m_sink, &QVideoSink::videoFrameChanged, this, &VideoSurface::onFrame);

    updateInterval();
    connect(Cfg::notifier(), &CfgNotifier::changed, this, [this] {
        updateInterval();
        update();
    });

    // часы на картинке обновляются раз в секунду, даже если кадры не приходят
    m_clock = new QTimer(this);
    m_clock->setInterval(1000);
    connect(m_clock, &QTimer::timeout, this, [this] {
        if (m_showTime && Cfg::view().timestamp && isVisible())
            update();
    });
    m_clock->start();
}

void VideoSurface::updateInterval()
{
    m_minIntervalMs = 1000 / qMax(1, Cfg::view().maxFps);
}

void VideoSurface::setShowTime(bool on)
{
    m_showTime = on;
    update();
}

void VideoSurface::setBrightness(int percent)
{
    m_brightness = qBound(-50, percent, 50);
    update();
}

void VideoSurface::setFillCell(bool on)
{
    if (m_fillCell == on) return;
    m_fillCell = on;
    update();
}

void VideoSurface::setStatusText(const QString &text)
{
    m_status = text;
    update();
}

void VideoSurface::setCameraRunning(bool running)
{
    if (m_cameraRunning == running)
        return;
    m_cameraRunning = running;
    update();
}

void VideoSurface::setLinkQuality(LinkQuality q)
{
    if (m_quality == q)
        return;
    m_quality = q;
    update();
}

void VideoSurface::resetZoom()
{
    m_view = QRectF(0.0, 0.0, 1.0, 1.0);
    update();
}

void VideoSurface::clearFrame()
{
    m_frame = QImage();
    m_view = QRectF(0.0, 0.0, 1.0, 1.0);
    m_lastFrameMs = 0;
    m_fps = 0;
    m_fpsFrameCount = 0;
    m_fpsWindowStartMs = 0;
    m_decodedMbps = 0.0;
    setLinkQuality(LinkQuality::None);
    update();
}

void VideoSurface::onFrame(const QVideoFrame &frame)
{
    // Лишние кадры отбрасываем до дорогого преобразования в картинку
    if (!frame.isValid() || !isVisible())
        return;
    const QWidget *w = window();
    if (w && w->isMinimized())
        return;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    setLinkQuality(LinkQuality::Good);   // кадр от камеры пришёл — связь есть, даже если он будет пропущен ниже
    if (!m_status.isEmpty()) {
        m_status.clear();
        update();
    }

    // FPS считаем по всем пришедшим от камеры кадрам, до ограничения частоты отрисовки ниже:
    // иначе в режиме для слабого компьютера счётчик показывал бы не поток камеры, а частоту экрана
    if (m_fpsWindowStartMs == 0)
        m_fpsWindowStartMs = now;
    ++m_fpsFrameCount;
    if (now - m_fpsWindowStartMs >= 1000) {
        m_fps = m_fpsFrameCount;
        // QMediaPlayer не предоставляет сетевой битрейт RTSP. Показываем
        // оценку объёма декодированного изображения в секунду, не выдавая её
        // за реальную скорость передачи по сети.
        const qint64 frameBytes = m_frame.isNull() ? 0 : m_frame.sizeInBytes();
        m_decodedMbps = (frameBytes > 0 && m_fps > 0)
            ? (double(frameBytes) * double(m_fps) * 8.0 / 1000000.0) : 0.0;
        m_fpsFrameCount = 0;
        m_fpsWindowStartMs = now;
        update();
    }

    if (now - m_lastFrameMs < m_minIntervalMs)
        return;                          // отрисовываем реже, чем приходят кадры (экономия для слабых ПК)
    // Конвертируем кадр в собственный непрерывный буфер. Некоторые аппаратные
    // декодеры переиспользуют память исходного кадра; хранение такой QImage без
    // detach/copy может давать полосы, tearing и «волну» при смене RTSP-потока.
    QImage img = frame.toImage();
    if (img.isNull())
        return;
    img = img.convertToFormat(QImage::Format_ARGB32_Premultiplied).copy();
    if (img.isNull())
        return;

    // Убираем гребёнку у явно помеченных чересстрочных кадров: используем одно
    // поле и восстанавливаем пропущенные строки интерполяцией соседних строк.
    // Qt 6.8 QVideoFrame does not expose fieldType()/InterlacedFrame.
    // Keep the converted frame unchanged; this avoids calling an API unavailable in Qt 6.
    m_lastFrameMs = now;
    m_frame = std::move(img);
    m_status.clear();
    update();
}

QRectF VideoSurface::targetRect() const
{
    QSizeF s = QSizeF(m_frame.size());
    s.scale(QSizeF(size()), m_fillCell ? Qt::KeepAspectRatioByExpanding : Qt::KeepAspectRatio);
    return QRectF((width() - s.width()) / 2.0, (height() - s.height()) / 2.0, s.width(), s.height());
}

void VideoSurface::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(0x12, 0x12, 0x13));
    const Cfg::View &v = Cfg::view();

    if (!m_frame.isNull()) {
        p.setRenderHint(QPainter::SmoothPixmapTransform, v.smooth);
        const QRectF dst = targetRect();
        const QSizeF fs(m_frame.size());
        const QRectF src(m_view.x() * fs.width(), m_view.y() * fs.height(),
                         m_view.width() * fs.width(), m_view.height() * fs.height());
        p.drawImage(dst, m_frame, src);

        if (m_brightness != 0) {
            const int level = qAbs(m_brightness) * 255 / 100;
            if (m_brightness > 0) {
                p.setCompositionMode(QPainter::CompositionMode_Plus);
                p.fillRect(dst, QColor(level, level, level));
            } else {
                p.setCompositionMode(QPainter::CompositionMode_Multiply);
                p.fillRect(dst, QColor(255 - level, 255 - level, 255 - level));
            }
            p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        }
    } else if (!m_status.isEmpty()) {
        p.setPen(QColor(0x6a, 0x6a, 0x6e));
        p.drawText(rect(), Qt::AlignCenter, m_status);
    }

    QFont f = p.font();
    f.setPixelSize(12);
    f.setBold(true);
    p.setFont(f);

    // дата и время поверх картинки
    if (m_showTime && v.timestamp && width() >= 120) {
        const QString t = QDateTime::currentDateTime().toString(v.timeFormat);
        const QFontMetrics fm(f);
        const QSize ts = fm.size(Qt::TextSingleLine, t) + QSize(16, 8);
        const QRect box(width() - ts.width() - 8, height() - ts.height() - 8, ts.width(), ts.height());
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 150));
        p.drawRoundedRect(box, 4, 4);
        p.setPen(QColor(0xf2, 0xf2, 0xf2));
        p.drawText(box, Qt::AlignCenter, t);
    }

    // панель сведений: качество связи, FPS и разрешение (можно скрыть в настройках)
    if (v.showInfo && width() >= 140) {
        QColor dot;
        switch (m_quality) {
        case LinkQuality::Good:       dot = QColor(0x3f, 0xb9, 0x50); break;
        case LinkQuality::Connecting: dot = QColor(0xe8, 0x83, 0x3a); break;
        case LinkQuality::None:       dot = QColor(0xd9, 0x53, 0x4f); break;
        }
        QString info;
        if (!m_frame.isNull()) {
            info = QStringLiteral("%1×%2    %3 FPS    %4 Мбит/с*")
                .arg(m_frame.width()).arg(m_frame.height()).arg(m_fps)
                .arg(m_decodedMbps, 0, 'f', 1);
        }
        const QFontMetrics fm(f);
        const int textW = info.isEmpty() ? 0 : fm.horizontalAdvance(info);
        const int boxW = 12 + 10 + (textW > 0 ? 8 + textW : 0) + 8;
        const QRect box(8, 8, boxW, 24);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 150));
        p.drawRoundedRect(box, 4, 4);
        p.setBrush(dot);
        p.drawEllipse(QPointF(box.left() + 14, box.center().y()), 5, 5);
        if (!info.isEmpty()) {
            p.setPen(QColor(0xf2, 0xf2, 0xf2));
            p.drawText(QRect(box.left() + 26, box.top(), textW + 4, box.height()), Qt::AlignVCenter | Qt::AlignLeft, info);
        }
    }

    // Небольшой фирменный индикатор работающей камеры — всегда в правом верхнем углу.
    // Рисуется на поверхности видео, поэтому остаётся на месте и в полноэкранном режиме.
    if (m_cameraRunning && width() >= 48 && height() >= 36) {
        const QRect badge(width() - 42, 8, 34, 27);
        p.setPen(QPen(QColor(0xb7, 0x9b, 0x55, 210), 1));
        p.setBrush(QColor(24, 25, 27, 205));
        p.drawRoundedRect(badge, 5, 5);

        // Камера: серо-стальной корпус с приглушёнными золотыми гранями.
        const qreal cx = badge.center().x();
        const qreal cy = badge.center().y();
        p.setPen(QPen(QColor(0xd0, 0xc4, 0xa1), 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.setBrush(QColor(0x9a, 0x91, 0x7a));
        QPainterPath body;
        body.moveTo(cx - 9, cy - 5);
        body.lineTo(cx + 3, cy - 5);
        body.lineTo(cx + 6, cy - 2);
        body.lineTo(cx + 6, cy + 5);
        body.lineTo(cx - 9, cy + 5);
        body.closeSubpath();
        p.drawPath(body);
        QPolygonF lens;
        lens << QPointF(cx + 6, cy - 3) << QPointF(cx + 11, cy - 6)
             << QPointF(cx + 11, cy + 6) << QPointF(cx + 6, cy + 3);
        p.setBrush(QColor(0xb9, 0xa2, 0x69));
        p.drawPolygon(lens);
        p.setPen(QPen(QColor(0x3a, 0x3b, 0x3d), 1));
        p.setBrush(QColor(0x34383c));
        p.drawEllipse(QPointF(cx - 3, cy), 2.2, 2.2);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0xf2, 0xc2, 0x00));
        p.drawEllipse(QPointF(badge.right() - 5, badge.top() + 5), 2.1, 2.1);
    }

    // значок масштаба — под индикатором камеры, чтобы элементы не перекрывались
    if (m_view.width() < 0.999) {
        const QString z = QStringLiteral("×%1").arg(1.0 / m_view.width(), 0, 'f', 1);
        const QRect box(width() - 62, 40, 54, 22);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0xf2, 0xc2, 0x00, 220));
        p.drawRoundedRect(box, 4, 4);
        p.setPen(QColor(0x1a, 0x1a, 0x1c));
        p.drawText(box, Qt::AlignCenter, z);
    }
}

void VideoSurface::wheelEvent(QWheelEvent *e)
{
    const double steps = e->angleDelta().y() / 120.0;
    if (m_frame.isNull() || steps == 0.0) {
        e->ignore();
        return;
    }
    const QRectF dst = targetRect();
    const QPointF pos = e->position();
    const double fx = qBound(0.0, (pos.x() - dst.x()) / dst.width(), 1.0);
    const double fy = qBound(0.0, (pos.y() - dst.y()) / dst.height(), 1.0);
    const double cx = m_view.x() + fx * m_view.width();     // точка кадра под курсором
    const double cy = m_view.y() + fy * m_view.height();

    double zoom = 1.0 / m_view.width();
    zoom = qBound(1.0, zoom * std::pow(1.2, steps), 8.0);
    const double w = 1.0 / zoom;
    const double x = qBound(0.0, cx - fx * w, 1.0 - w);     // точка под курсором остаётся на месте
    const double y = qBound(0.0, cy - fy * w, 1.0 - w);
    m_view = QRectF(x, y, w, w);
    update();
    e->accept();
}

void VideoSurface::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        emit clicked();
        if (m_view.width() < 0.999) {
            m_panning = true;
            m_panStart = e->pos();
            m_panView = m_view;
            setCursor(Qt::ClosedHandCursor);
        }
    } else if (e->button() == Qt::MiddleButton) {
        resetZoom();
    }
    QWidget::mousePressEvent(e);
}

void VideoSurface::mouseMoveEvent(QMouseEvent *e)
{
    if (m_panning && !m_frame.isNull()) {
        const QRectF dst = targetRect();
        const double dx = (e->pos().x() - m_panStart.x()) / dst.width() * m_panView.width();
        const double dy = (e->pos().y() - m_panStart.y()) / dst.height() * m_panView.height();
        const double w = m_panView.width();
        m_view = QRectF(qBound(0.0, m_panView.x() - dx, 1.0 - w), qBound(0.0, m_panView.y() - dy, 1.0 - w), w, w);
        update();
    }
    QWidget::mouseMoveEvent(e);
}

void VideoSurface::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_panning) {
        m_panning = false;
        unsetCursor();
    }
    QWidget::mouseReleaseEvent(e);
}

void VideoSurface::mouseDoubleClickEvent(QMouseEvent *e)
{
    emit doubleClicked();
    QWidget::mouseDoubleClickEvent(e);
}

void VideoSurface::contextMenuEvent(QContextMenuEvent *e)
{
    emit contextMenuRequested(e->globalPos());
}

// ======================================================================
// core/app_paths.cpp
// ======================================================================
namespace {

QString computeDataDir()
{
    const QString preferred = QCoreApplication::applicationDirPath() + QStringLiteral("/data");
    QDir().mkpath(preferred);
    // Проверяем, что в папку программы реально можно писать (бывает нельзя — например,
    // в Program Files без прав администратора); если нет, используем папку пользователя.
    QFile probe(preferred + QStringLiteral("/.write_test"));
    if (probe.open(QIODevice::WriteOnly)) {
        probe.close();
        probe.remove();
        return preferred;
    }
    const QString fallback = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(fallback);
    return fallback;
}

} // namespace

QString AppPaths::dataDir()
{
    static const QString dir = computeDataDir();
    return dir;
}

void AppPaths::initSettingsLocation()
{
    // Настройки — в обычном читаемом INI-файле рядом с программой, а не в реестре Windows:
    // так весь профиль программы (настройки + камеры + архив) переносится на другой компьютер
    // простым копированием папки.
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, dataDir());
    QSettings::setDefaultFormat(QSettings::IniFormat);
}

// ======================================================================
// core/backup.cpp
// ======================================================================
namespace {

bool isBackupKey(const QString &key)
{
    if (key == QLatin1String("ui/geometry"))
        return false;
    static const char *const prefixes[] = {"cameras/", "layout/", "templates/", "general/",
                                           "storage/", "perf/", "view/", "ui/", "cloud/"};
    for (const char *p : prefixes)
        if (key.startsWith(QLatin1String(p)))
            return true;
    return false;
}

QVariant backupValueFromJson(const QJsonValue &v)
{
    if (v.isArray()) {
        QStringList list;
        const QJsonArray arr = v.toArray();
        for (const QJsonValue &x : arr)
            list << x.toVariant().toString();
        return list;
    }
    if (v.isDouble()) {
        const double d = v.toDouble();
        if (d == double(qint64(d)))
            return qint64(d);
    }
    return v.toVariant();
}

} // namespace

bool Backup::exportTo(const QString &path, QString *error)
{
    QSettings s;
    QJsonObject keys;
    const QStringList all = s.allKeys();
    for (const QString &k : all) {
        if (isBackupKey(k))
            keys.insert(k, QJsonValue::fromVariant(s.value(k)));
    }
    QJsonObject root;
    root.insert(QStringLiteral("app"), QStringLiteral("PSVMS"));
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("created"), QDateTime::currentDateTime().toString(Qt::ISODate));
    root.insert(QStringLiteral("settings"), keys);

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error) *error = T("Не удалось записать файл: %1").arg(f.errorString());
        return false;
    }
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

bool Backup::importFrom(const QString &path, QString *error)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        if (error) *error = T("Не удалось прочитать файл: %1").arg(f.errorString());
        return false;
    }
    QJsonParseError pe;
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &pe);
    if (pe.error != QJsonParseError::NoError || !doc.isObject()
        || doc.object().value(QStringLiteral("app")).toString() != QLatin1String("PSVMS")) {
        if (error) *error = T("Файл повреждён или не является резервной копией программы.");
        return false;
    }
    const QJsonObject keys = doc.object().value(QStringLiteral("settings")).toObject();

    QSettings s;
    static const char *const groups[] = {"cameras", "layout", "templates", "general",
                                         "storage", "perf", "view", "cloud"};
    for (const char *g : groups)
        s.remove(QString::fromLatin1(g));
    for (auto it = keys.begin(); it != keys.end(); ++it) {
        if (isBackupKey(it.key()))
            s.setValue(it.key(), backupValueFromJson(it.value()));
    }
    s.sync();
    return true;
}

// ======================================================================
// core/camera_worker.cpp
// ======================================================================
namespace {

QString avErr(int code)
{
    char buf[AV_ERROR_MAX_STRING_SIZE] = {};
    av_strerror(code, buf, sizeof buf);
    return QString::fromUtf8(buf);
}

// Пишет один mkv-сегмент: копирует видео (и аудио, если mkv его поддерживает)
// из входного потока, сдвигая метки времени так, чтобы сегмент начинался с 0.
class SegmentWriter {
public:
    ~SegmentWriter() { close(); }

    bool isOpen() const { return m_oc != nullptr; }

    bool open(const QString &path, const AVFormatContext *in, QString *err)
    {
        m_path = path;
        const QByteArray fn = path.toUtf8();   // FFmpeg на Windows ждёт UTF-8 (в пути может быть кириллица)

        int r = avformat_alloc_output_context2(&m_oc, nullptr, "matroska", fn.constData());
        if (r < 0 || !m_oc)
            return fail(err, QStringLiteral("alloc_output_context2"), r < 0 ? r : AVERROR(ENOMEM));

        m_map.assign(in->nb_streams, -1);
        m_lastDts.assign(in->nb_streams, INT64_MIN);

        for (unsigned i = 0; i < in->nb_streams; ++i) {
            const AVCodecParameters *par = in->streams[i]->codecpar;
            if (par->codec_type != AVMEDIA_TYPE_VIDEO && par->codec_type != AVMEDIA_TYPE_AUDIO)
                continue;
            // экзотический аудиокодек, который mkv не принимает: пишем только видео
            if (par->codec_type == AVMEDIA_TYPE_AUDIO
                && avformat_query_codec(m_oc->oformat, par->codec_id, FF_COMPLIANCE_NORMAL) != 1)
                continue;

            AVStream *os = avformat_new_stream(m_oc, nullptr);
            if (!os)
                return fail(err, QStringLiteral("new_stream"), AVERROR(ENOMEM));
            r = avcodec_parameters_copy(os->codecpar, par);
            if (r < 0)
                return fail(err, QStringLiteral("parameters_copy"), r);
            os->codecpar->codec_tag = 0;
            m_map[i] = os->index;
        }

        r = avio_open(&m_oc->pb, fn.constData(), AVIO_FLAG_WRITE);
        if (r < 0)
            return fail(err, QStringLiteral("avio_open"), r);
        r = avformat_write_header(m_oc, nullptr);
        if (r < 0)
            return fail(err, QStringLiteral("write_header"), r);
        m_headerWritten = true;
        return true;
    }

    // 0 — пакет записан или пропущен, <0 — ошибка записи.
    // Пакет после вызова больше не нужен (av_packet_unref вызывать можно).
    int write(AVPacket *pkt, const AVStream *in)
    {
        const int idx = pkt->stream_index;
        if (idx < 0 || idx >= int(m_map.size()) || m_map[idx] < 0)
            return 0;
        const int o = m_map[idx];
        AVStream *os = m_oc->streams[o];

        int64_t dts = pkt->dts, pts = pkt->pts;
        if (dts == AV_NOPTS_VALUE) dts = pts;
        if (pts == AV_NOPTS_VALUE) pts = dts;
        if (dts == AV_NOPTS_VALUE) return 0;

        // Точка отсчёта сегмента — dts первого записанного пакета (ключевой кадр видео).
        if (m_baseTs == AV_NOPTS_VALUE) {
            m_baseTs = dts;
            m_baseTb = in->time_base;
        }
        const int64_t off = av_rescale_q(m_baseTs, m_baseTb, in->time_base);
        dts -= off;
        pts -= off;
        if (dts < 0)
            return 0;   // аудио, пришедшее раньше первого ключевого кадра

        dts = av_rescale_q(dts, in->time_base, os->time_base);
        pts = av_rescale_q(pts, in->time_base, os->time_base);
        const int64_t dur = pkt->duration > 0
            ? av_rescale_q(pkt->duration, in->time_base, os->time_base) : 0;

        // mkv требует неубывающих dts; RTSP-камеры иногда присылают повторы
        if (dts <= m_lastDts[o]) dts = m_lastDts[o] + 1;
        if (pts < dts) pts = dts;
        m_lastDts[o] = dts;

        pkt->pts = pts;
        pkt->dts = dts;
        pkt->duration = dur;
        pkt->stream_index = o;
        pkt->pos = -1;
        return av_interleaved_write_frame(m_oc, pkt);
    }

    // Закрывает файл, возвращает его размер в байтах.
    qint64 close()
    {
        if (!m_oc)
            return 0;
        if (m_headerWritten)
            av_write_trailer(m_oc);
        if (m_oc->pb)
            avio_closep(&m_oc->pb);
        avformat_free_context(m_oc);
        m_oc = nullptr;
        m_headerWritten = false;
        m_baseTs = AV_NOPTS_VALUE;
        return QFileInfo(m_path).size();
    }

private:
    bool fail(QString *err, const QString &what, int code)
    {
        if (err)
            *err = what + QStringLiteral(": ") + avErr(code);
        if (m_oc) {
            if (m_oc->pb)
                avio_closep(&m_oc->pb);
            avformat_free_context(m_oc);
            m_oc = nullptr;
        }
        m_headerWritten = false;
        return false;
    }

    AVFormatContext *m_oc = nullptr;
    QString m_path;
    bool m_headerWritten = false;
    std::vector<int> m_map;          // индекс входного потока -> индекс выходного (или -1)
    std::vector<int64_t> m_lastDts;  // по выходным потокам
    int64_t m_baseTs = AV_NOPTS_VALUE;
    AVRational m_baseTb = {1, 1};
};

} // namespace

// ---------------------------------------------------------------------------

CameraWorker::CameraWorker(CameraConfig cfg, QObject *parent)
    : QThread(parent), m_cfg(std::move(cfg))
{
}

CameraWorker::~CameraWorker()
{
    stop();
    wait();
}

void CameraWorker::startRecording()
{
    m_stop = false;
    start();
}

void CameraWorker::stop()
{
    m_stop = true;
}

int CameraWorker::interruptCallback(void *opaque)
{
    auto *self = static_cast<CameraWorker *>(opaque);
    if (self->m_stop)
        return 1;
    const qint64 deadline = self->m_deadlineMs.load();
    return (deadline > 0 && QDateTime::currentMSecsSinceEpoch() > deadline) ? 1 : 0;
}

void CameraWorker::armDeadline(int ms)
{
    m_deadlineMs = QDateTime::currentMSecsSinceEpoch() + ms;
}

void CameraWorker::sleepInterruptible(int ms)
{
    for (int waited = 0; waited < ms && !m_stop; waited += 100)
        msleep(100);
}

QString CameraWorker::segmentPath(qint64 startMs) const
{
    const QDateTime t = QDateTime::fromMSecsSinceEpoch(startMs);
    const QString dir = QStringLiteral("%1/%2/%3")
                            .arg(m_cfg.storageRoot, m_cfg.id, t.toString(QStringLiteral("yyyy/MM/dd")));
    QDir().mkpath(dir);
    return QStringLiteral("%1/%2_%3.mkv")
        .arg(dir, m_cfg.id, t.toString(QStringLiteral("yyyyMMdd_HHmmss")));
}

void CameraWorker::run()
{
    // Соединение с БД должно жить в том же потоке, где используется.
    RecordingIndex index;
    if (!index.open(m_cfg.dbPath)) {
        emit errorOccurred(m_cfg.id, T("Не удалось открыть БД: %1").arg(index.lastError()));
        emit stateChanged(m_cfg.id, State::Stopped);
        return;
    }
    index.closeDanglingSegments(m_cfg.id);

    int backoffMs = 1000;
    while (!m_stop) {
        emit stateChanged(m_cfg.id, State::Connecting);

        bool gotData = false;
        const qint64 sessionStartedMs = QDateTime::currentMSecsSinceEpoch();
        recordSession(index, gotData);
        if (m_stop)
            break;

        emit stateChanged(m_cfg.id, State::Reconnecting);
        // Не сбрасываем задержку только из-за одного полученного пакета:
        // нестабильные камеры иначе вызывают частые циклы подключения.
        const qint64 sessionDurationMs = QDateTime::currentMSecsSinceEpoch() - sessionStartedMs;
        if (gotData && sessionDurationMs >= 30000)
            backoffMs = 1000;
        else
            backoffMs = qMin(backoffMs * 2, 30000);
        sleepInterruptible(backoffMs);
    }
    emit stateChanged(m_cfg.id, State::Stopped);
}

// Одна сессия: подключились -> читаем пакеты -> пишем сегменты -> вышли при обрыве или stop().
void CameraWorker::recordSession(RecordingIndex &index, bool &gotData)
{
    AVFormatContext *ic = avformat_alloc_context();
    if (!ic) {
        emit errorOccurred(m_cfg.id, T("Не хватает памяти"));
        return;
    }
    ic->interrupt_callback.callback = &CameraWorker::interruptCallback;
    ic->interrupt_callback.opaque = this;

    AVDictionary *opts = nullptr;
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    const QByteArray ioTimeout = QByteArray::number(qint64(m_cfg.ioTimeoutMs) * 1000);
    // Разные сборки FFmpeg используют разные имена таймаута RTSP/I/O.
    av_dict_set(&opts, "timeout", ioTimeout.constData(), 0);
    av_dict_set(&opts, "stimeout", ioTimeout.constData(), 0);
    av_dict_set(&opts, "rw_timeout", ioTimeout.constData(), 0);

    armDeadline(m_cfg.ioTimeoutMs);
    int r = avformat_open_input(&ic, m_cfg.url.toUtf8().constData(), nullptr, &opts);
    av_dict_free(&opts);
    if (r < 0) {   // при ошибке ic уже освобождён
        emit errorOccurred(m_cfg.id, T("Не удалось подключиться: %1").arg(avErr(r)));
        return;
    }
    auto closeInput = qScopeGuard([&] { avformat_close_input(&ic); });

    armDeadline(m_cfg.ioTimeoutMs * 2);
    r = avformat_find_stream_info(ic, nullptr);
    if (r < 0) {
        emit errorOccurred(m_cfg.id, T("Не удалось получить параметры потока: %1").arg(avErr(r)));
        return;
    }
    const int videoIdx = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (videoIdx < 0) {
        emit errorOccurred(m_cfg.id, T("В потоке нет видео"));
        return;
    }

    AVPacket *pkt = av_packet_alloc();
    auto freePkt = qScopeGuard([&] { av_packet_free(&pkt); });

    SegmentWriter writer;
    QString segPath;
    qint64 segId = -1, segStartMs = 0, lastPktMs = 0;
    bool announcedRecording = false;

    auto finishSegment = [&] {
        if (!writer.isOpen())
            return;
        const qint64 size = writer.close();
        if (segId >= 0)
            index.endSegment(segId, lastPktMs, size);
        emit segmentFinished(m_cfg.id, segPath, segStartMs, lastPktMs, size);
        segId = -1;
    };
    // Закрыть сегмент при любом выходе из функции (обрыв, ошибка, stop)
    auto closeSegment = qScopeGuard(finishSegment);

    const qint64 segmentMs = qint64(m_cfg.segmentSeconds) * 1000;

    while (!m_stop) {
        armDeadline(m_cfg.ioTimeoutMs);
        r = av_read_frame(ic, pkt);
        if (r < 0) {
            if (!m_stop)
                emit errorOccurred(m_cfg.id, T("Поток прерван: %1").arg(avErr(r)));
            break;
        }
        gotData = true;

        const bool isKey = pkt->stream_index == videoIdx && (pkt->flags & AV_PKT_FLAG_KEY);
        const qint64 now = QDateTime::currentMSecsSinceEpoch();

        // Ротация только на ключевом кадре, чтобы каждый сегмент проигрывался сам по себе
        if (writer.isOpen() && isKey && now - segStartMs >= segmentMs)
            finishSegment();

        if (!writer.isOpen()) {
            if (!isKey) {                       // ждём ключевой кадр
                av_packet_unref(pkt);
                continue;
            }
            segStartMs = now;
            segPath = segmentPath(segStartMs);
            QString err;
            if (!writer.open(segPath, ic, &err)) {
                emit errorOccurred(m_cfg.id, T("Не удалось создать сегмент: %1").arg(err));
                av_packet_unref(pkt);
                break;
            }
            segId = index.beginSegment(m_cfg.id, segPath, segStartMs);
            if (segId < 0)
                emit errorOccurred(m_cfg.id, T("Ошибка индекса: %1").arg(index.lastError()));
            emit segmentStarted(m_cfg.id, segPath, segStartMs);
            if (!announcedRecording) {
                announcedRecording = true;
                emit stateChanged(m_cfg.id, State::Recording);
            }
        }

        lastPktMs = now;
        // Сюда же позже: копия пакета в декодер для живого просмотра и аналитики.
        const int wr = writer.write(pkt, ic->streams[pkt->stream_index]);
        av_packet_unref(pkt);
        if (wr < 0) {
            emit errorOccurred(m_cfg.id, T("Ошибка записи (диск?): %1").arg(avErr(wr)));
            break;
        }
    }
}

// ======================================================================
// core/device_adder.cpp
// ======================================================================
DeviceAdder::DeviceAdder(QObject *parent) : QObject(parent) {}

void DeviceAdder::start(const QList<AddRequest> &requests, int concurrency)
{
    ++m_gen;
    m_reqs = requests;
    m_next = 0;
    m_done = 0;
    m_active = 0;
    m_conc = qMax(1, concurrency);
    m_running = true;
    if (requests.isEmpty()) {
        m_running = false;
        emit finished();
        return;
    }
    pump();
}

void DeviceAdder::cancel()
{
    m_running = false;
    ++m_gen;
}

void DeviceAdder::pump()
{
    while (m_running && m_active < m_conc && m_next < m_reqs.size()) {
        ++m_active;
        runOne(m_next++);
    }
}

void DeviceAdder::finishOne(int idx, const AddResult &r, int gen)
{
    if (gen != m_gen)
        return;
    --m_active;
    ++m_done;
    emit resolved(idx, r);
    emit progress(m_done, int(m_reqs.size()));
    if (m_done >= m_reqs.size()) {
        m_running = false;
        emit finished();
    } else {
        pump();
    }
}

void DeviceAdder::runOne(int idx)
{
    const AddRequest rq = m_reqs.at(idx);
    const int gen = m_gen;

    auto *hr = new HikResolver(this);
    connect(hr, &HikResolver::resolved, this, [this, idx, rq, gen, hr](int, const HikResult &r) {
        hr->deleteLater();
        if (gen != m_gen)
            return;
        if (r.ok) {
            const HikDevice &d = r.device;
            AddResult res;
            res.ok = true;
            res.method = QStringLiteral("ISAPI");
            res.model = d.model;
            res.name = d.name;
            res.type = d.type;
            res.recorder = d.isRecorder();
            for (const HikChannel &c : d.channels) {
                AddedChannel ch;
                ch.number = c.number;
                ch.name = c.name;
                ch.mainUrl = hikRtspUrl(rq.address, rq.rtspPort, rq.user, rq.password, c.number, 1);
                ch.subUrl = hikRtspUrl(rq.address, rq.rtspPort, rq.user, rq.password, c.number, 2);
                res.channels.append(ch);
            }
            finishOne(idx, res, gen);
        } else if (r.authFailed) {
            AddResult res;
            res.authFailed = true;
            res.error = r.error;
            finishOne(idx, res, gen);
        } else {
            probeRtsp(idx, rq, r.error, gen);   // старая прошивка или другой бренд
        }
    });

    HikRequest hq;
    hq.address = rq.address;
    hq.httpPort = rq.httpPort;
    hq.user = rq.user;
    hq.password = rq.password;
    hq.fallbackChannels = rq.channels;
    hr->start({hq}, 1);
}

void DeviceAdder::probeRtsp(int idx, const AddRequest &rq, const QString &prevError, int gen)
{
    auto *probe = new RtspProbe(this);
    connect(probe, &RtspProbe::finished, this,
            [this, idx, rq, prevError, gen, probe](bool ok, int tmpl, const QString &err) {
        probe->deleteLater();
        if (gen != m_gen)
            return;
        AddResult res;
        if (!ok) {
            res.authFailed = err == QLatin1String("auth");
            res.error = res.authFailed ? T("Неверный логин или пароль")
                                       : (prevError.isEmpty() ? T("Не удалось определить адрес потока камеры") : prevError);
            finishOne(idx, res, gen);
            return;
        }
        res.ok = true;
        res.method = QStringLiteral("RTSP");
        const int n = qMax(1, rq.channels);
        res.recorder = n > 1;
        for (int ch = 1; ch <= n; ++ch) {
            AddedChannel c;
            c.number = ch;
            c.mainUrl = RtspProbe::buildUrl(tmpl, rq.address, rq.rtspPort, rq.user, rq.password, ch, false);
            c.subUrl = RtspProbe::buildUrl(tmpl, rq.address, rq.rtspPort, rq.user, rq.password, ch, true);
            res.channels.append(c);
        }
        finishOne(idx, res, gen);
    });
    probe->start(rq.address, rq.rtspPort, rq.user, rq.password);
}

// ======================================================================
// core/hikvision.cpp
// ======================================================================
namespace {

bool isName_hik(const QXmlStreamReader &x, const char *n)
{
    return x.name() == QLatin1String(n);
}

struct HttpReply {
    bool ok = false;
    bool authFailed = false;
    QByteArray body;
    QString error;
};

// GET с HTTP Digest: логин и пароль берутся из свойств ответа (разные для каждого устройства)
void httpGet(QNetworkAccessManager *nam, QObject *ctx, const HikRequest &req, const QString &path,
             std::function<void(const HttpReply &)> cb)
{
    QUrl url;
    url.setScheme(QStringLiteral("http"));
    url.setHost(req.address);
    url.setPort(req.httpPort);
    url.setPath(path);

    if (url.scheme().compare(QLatin1String("https"), Qt::CaseInsensitive) != 0 && !req.password.isEmpty())
        SecurityAudit::write(QStringLiteral("camera_http_credentials_warning"), req.address);
    QNetworkRequest nr(url);
    if (url.scheme().compare(QLatin1String("https"), Qt::CaseInsensitive) == 0) {
        QSslConfiguration ssl = QSslConfiguration::defaultConfiguration();
        ssl.setPeerVerifyMode(QSslSocket::VerifyPeer);
        nr.setSslConfiguration(ssl);
    }
    nr.setTransferTimeout(5000);
    QNetworkReply *reply = nam->get(nr);
    reply->setProperty("hik_user", req.user);
    reply->setProperty("hik_pass", req.password);

    QObject::connect(reply, &QNetworkReply::finished, ctx, [reply, cb] {
        HttpReply h;
        h.body = reply->readAll();
        h.ok = reply->error() == QNetworkReply::NoError;
        h.authFailed = reply->error() == QNetworkReply::AuthenticationRequiredError;
        if (!h.ok) {
            switch (reply->error()) {
            case QNetworkReply::AuthenticationRequiredError:
                h.error = T("Неверный логин или пароль (или устройство не активировано)");
                break;
            case QNetworkReply::TimeoutError:
                h.error = T("Не отвечает (таймаут)");
                break;
            case QNetworkReply::ConnectionRefusedError:
                h.error = T("Порт закрыт");
                break;
            case QNetworkReply::ContentNotFoundError:
                h.error = T("Это не Hikvision/HiWatch (нет ISAPI)");
                break;
            default:
                h.error = reply->errorString();
            }
        }
        reply->deleteLater();
        cb(h);
    });
}

void parseDeviceInfo(const QByteArray &xml, HikDevice *dev)
{
    QXmlStreamReader x(xml);
    while (!x.atEnd()) {
        x.readNext();
        if (!x.isStartElement())
            continue;
        if (isName_hik(x, "deviceName")) dev->name = x.readElementText().trimmed();
        else if (isName_hik(x, "model")) dev->model = x.readElementText().trimmed();
        else if (isName_hik(x, "serialNumber")) dev->serial = x.readElementText().trimmed();
        else if (isName_hik(x, "deviceType")) dev->type = x.readElementText().trimmed();
    }
}

// Каналы из /ISAPI/Streaming/channels: id 101 и 102 — канал 1, 201 и 202 — канал 2 и т. д.
QList<HikChannel> parseChannels(const QByteArray &xml)
{
    QMap<int, QString> found;
    QXmlStreamReader x(xml);
    bool inCh = false;
    int id = 0;
    QString name;
    while (!x.atEnd()) {
        x.readNext();
        if (x.isStartElement()) {
            if (isName_hik(x, "StreamingChannel")) {
                inCh = true;
                id = 0;
                name.clear();
            } else if (inCh && id == 0 && isName_hik(x, "id")) {
                id = x.readElementText().toInt();
            } else if (inCh && name.isEmpty() && isName_hik(x, "channelName")) {
                name = x.readElementText().trimmed();
            }
        } else if (x.isEndElement() && isName_hik(x, "StreamingChannel")) {
            if (id >= 101 && (id % 100 == 1 || id % 100 == 2)) {
                const int n = id / 100;
                if (!found.contains(n) || found.value(n).isEmpty())
                    found[n] = name;
            }
            inCh = false;
        }
    }
    QList<HikChannel> out;
    for (auto it = found.cbegin(); it != found.cend(); ++it) {
        HikChannel c;
        c.number = it.key();
        c.name = it.value();
        out.append(c);
    }
    return out;
}

} // namespace

// ===========================================================================
// HikDiscovery (SADP)
// ===========================================================================

HikDiscovery::HikDiscovery(QObject *parent) : QObject(parent)
{
    m_socket = new QUdpSocket(this);
    connect(m_socket, &QUdpSocket::readyRead, this, &HikDiscovery::onReadyRead);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, [this] {
        m_running = false;
        emit finished();
    });
}

void HikDiscovery::start(int timeoutMs)
{
    stop();
    m_seen.clear();

    // Устройства отвечают на multicast-группу, поэтому слушаем порт 37020 и входим в группу
    if (m_socket->state() != QAbstractSocket::BoundState
        && !m_socket->bind(QHostAddress::AnyIPv4, 37020,
                           QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        emit finished();
        return;
    }
    const QHostAddress group(QStringLiteral("239.255.255.250"));
    for (const QNetworkInterface &ni : QNetworkInterface::allInterfaces()) {
        const auto fl = ni.flags();
        if ((fl & QNetworkInterface::IsUp) && (fl & QNetworkInterface::IsRunning)
            && (fl & QNetworkInterface::CanMulticast) && !(fl & QNetworkInterface::IsLoopBack))
            m_socket->joinMulticastGroup(group, ni);
    }
    m_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 4);

    m_running = true;
    sendProbes();
    QTimer::singleShot(1000, this, [this] { if (m_running) sendProbes(); });
    m_timer->start(timeoutMs);
}

void HikDiscovery::stop()
{
    m_running = false;
    m_timer->stop();
}

void HikDiscovery::sendProbes()
{
    const QByteArray probe =
        QByteArray("<?xml version=\"1.0\" encoding=\"utf-8\"?><Probe><Uuid>")
        + QUuid::createUuid().toString(QUuid::WithoutBraces).toUpper().toUtf8()
        + QByteArray("</Uuid><Types>inquiry</Types></Probe>");

    const QHostAddress group(QStringLiteral("239.255.255.250"));
    for (const QNetworkInterface &ni : QNetworkInterface::allInterfaces()) {
        const auto fl = ni.flags();
        if (!(fl & QNetworkInterface::IsUp) || !(fl & QNetworkInterface::IsRunning)
            || (fl & QNetworkInterface::IsLoopBack) || !(fl & QNetworkInterface::CanMulticast))
            continue;
        m_socket->setMulticastInterface(ni);
        m_socket->writeDatagram(probe, group, 37020);
    }
}

void HikDiscovery::onReadyRead()
{
    while (m_socket->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(int(m_socket->pendingDatagramSize()));
        m_socket->readDatagram(data.data(), data.size());
        if (!m_running || !data.contains("ProbeMatch"))   // свои же запросы пропускаем
            continue;

        HikDevice dev;
        QXmlStreamReader x(data);
        while (!x.atEnd()) {
            x.readNext();
            if (!x.isStartElement())
                continue;
            if (isName_hik(x, "IPv4Address")) dev.address = x.readElementText().trimmed();
            else if (isName_hik(x, "HttpPort")) dev.httpPort = quint16(x.readElementText().toUInt());
            else if (isName_hik(x, "DeviceDescription")) dev.model = x.readElementText().trimmed();
            else if (isName_hik(x, "DeviceSN")) dev.serial = x.readElementText().trimmed();
            else if (isName_hik(x, "Activated")) dev.activated = x.readElementText().trimmed() != QLatin1String("false");
        }
        if (dev.httpPort == 0)
            dev.httpPort = 80;
        if (dev.address.isEmpty() || m_seen.contains(dev.address))
            continue;
        m_seen.insert(dev.address);
        emit deviceFound(dev);
    }
}

// ===========================================================================
// HikResolver (ISAPI)
// ===========================================================================

HikResolver::HikResolver(QObject *parent) : QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);
    connect(m_nam, &QNetworkAccessManager::authenticationRequired, this,
            [](QNetworkReply *reply, QAuthenticator *auth) {
                auth->setUser(reply->property("hik_user").toString());
                auth->setPassword(reply->property("hik_pass").toString());
            });
}

void HikResolver::start(const QList<HikRequest> &requests, int concurrency)
{
    ++m_gen;
    m_requests = requests;
    m_next = 0;
    m_done = 0;
    m_active = 0;
    m_conc = qMax(1, concurrency);
    m_running = true;
    if (requests.isEmpty()) {
        m_running = false;
        emit finished();
        return;
    }
    pump();
}

void HikResolver::cancel()
{
    m_running = false;
    ++m_gen;
}

void HikResolver::pump()
{
    while (m_running && m_active < m_conc && m_next < m_requests.size()) {
        ++m_active;
        runJob(m_next++);
    }
}

void HikResolver::jobDone(int idx, const HikResult &r, int gen)
{
    if (gen != m_gen)
        return;
    --m_active;
    ++m_done;
    emit resolved(idx, r);
    emit progress(m_done, int(m_requests.size()));
    if (m_done >= m_requests.size()) {
        m_running = false;
        emit finished();
    } else {
        pump();
    }
}

void HikResolver::runJob(int idx)
{
    const HikRequest req = m_requests.at(idx);
    const int gen = m_gen;

    auto fail = [this, idx, gen](const QString &msg, bool auth = false) {
        HikResult r;
        r.error = msg;
        r.authFailed = auth;
        jobDone(idx, r, gen);
    };

    // 1. Тип устройства и модель
    httpGet(m_nam, this, req, QStringLiteral("/ISAPI/System/deviceInfo"), [=](const HttpReply &r1) {
        if (gen != m_gen)
            return;
        if (!r1.ok) {
            fail(r1.error, r1.authFailed);
            return;
        }
        HikDevice dev;
        dev.address = req.address;
        dev.httpPort = req.httpPort;
        parseDeviceInfo(r1.body, &dev);
        if (dev.model.isEmpty() && dev.name.isEmpty()) {
            fail(T("Ответ не похож на Hikvision/HiWatch"));
            return;
        }

        // 2. Каналы (у регистратора — все, у камеры — один)
        httpGet(m_nam, this, req, QStringLiteral("/ISAPI/Streaming/channels"), [=](const HttpReply &r2) {
            if (gen != m_gen)
                return;
            HikDevice d = dev;
            if (r2.ok)
                d.channels = parseChannels(r2.body);
            if (d.channels.isEmpty()) {
                const int n = d.isRecorder() ? req.fallbackChannels : 1;
                for (int i = 1; i <= n; ++i) {
                    HikChannel c;
                    c.number = i;
                    d.channels.append(c);
                }
            }
            if (d.channels.isEmpty()) {
                fail(T("Не удалось получить каналы регистратора: укажите их число вручную"));
                return;
            }
            HikResult res;
            res.ok = true;
            res.device = d;
            jobDone(idx, res, gen);
        });
    });
}

QString guessVendor(const QString &model, const QString &name)
{
    const QString s = (model + QLatin1Char(' ') + name).trimmed();
    if (s.isEmpty())
        return QString();

    static const QRegularExpression polyvision(QStringLiteral("POLY\\s*VISION|\\bPV[CDRN]-"),
                                               QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression hiwatch(QStringLiteral("HIWATCH|\\bHW[ICPDN]-|\\bDS-[ITNHD]\\d"),
                                            QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression hikvision(QStringLiteral("HIKVISION|\\bI?DS-"),
                                              QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression dahua(QStringLiteral("DAHUA|\\bDH-|\\bIPC-"),
                                          QRegularExpression::CaseInsensitiveOption);
    if (polyvision.match(s).hasMatch())
        return QStringLiteral("Polyvision");
    if (hiwatch.match(s).hasMatch())
        return QStringLiteral("HiWatch");
    if (hikvision.match(s).hasMatch())
        return QStringLiteral("Hikvision");
    if (dahua.match(s).hasMatch())
        return QStringLiteral("Dahua");
    return QString();
}

// ======================================================================
// core/i18n.cpp
// ======================================================================
namespace {

I18n::Lang g_lang = I18n::Lang::Ru;
bool g_restartRequested = false;

// Русский текст (ключ) -> английский перевод.
// Список строится из вызовов T("...") в коде; нового текста здесь нет — добавьте строку, чтобы перевести.
const QHash<QString, QString> &englishDict()
{
    static const QHash<QString, QString> dict = {
        {QStringLiteral("    %1 МБ"), QStringLiteral("    %1 MB")},
        {QStringLiteral(" (исправьте логин/пароль и повторите)"), QStringLiteral(" (fix the login/password and try again)")},
        {QStringLiteral(" — исправьте логин/пароль и повторите"), QStringLiteral(" — fix the login/password and try again")},
        {QStringLiteral("%1 — %2, каналов: %3"), QStringLiteral("%1 — %2, channels: %3")},
        {QStringLiteral("1. Адреса камер"), QStringLiteral("1. Camera addresses")},
        {QStringLiteral("1. Поиск камер"), QStringLiteral("1. Find cameras")},
        {QStringLiteral("1. Поиск устройств"), QStringLiteral("1. Find devices")},
        {QStringLiteral("2. Доступ"), QStringLiteral("2. Access")},
        {QStringLiteral("2. Логин и пароль камер"), QStringLiteral("2. Camera login and password")},
        {QStringLiteral("2. Общие логин и пароль камер"), QStringLiteral("2. Shared camera login and password")},
        {QStringLiteral("3. Найденные адреса с открытым RTSP-портом"), QStringLiteral("3. Addresses with an open RTSP port")},
        {QStringLiteral("3. Найденные устройства"), QStringLiteral("3. Devices found")},
        {QStringLiteral("HTTP-порт"), QStringLiteral("HTTP port")},
        {QStringLiteral("MiniVMS — вход"), QStringLiteral("MiniVMS — sign in")},
        {QStringLiteral("MiniVMS — создание пароля"), QStringLiteral("MiniVMS — create password")},
        {QStringLiteral("PTZ-управление"), QStringLiteral("PTZ control")},
        {QStringLiteral("RTSP-порт"), QStringLiteral("RTSP port")},
        {QStringLiteral("RTSP-порт камеры, по умолчанию 554"), QStringLiteral("Camera RTSP port, 554 by default")},
        {QStringLiteral("RTSP-порт открыт"), QStringLiteral("RTSP port is open")},
        {QStringLiteral("rtsp://логин:пароль@192.168.1.64:554/Streaming/Channels/101"), QStringLiteral("rtsp://login:password@192.168.1.64:554/Streaming/Channels/101")},
        {QStringLiteral("Адрес"), QStringLiteral("Address")},
        {QStringLiteral("Архив"), QStringLiteral("Archive")},
        {QStringLiteral("Без группы"), QStringLiteral("No group")},
        {QStringLiteral("В потоке нет видео"), QStringLiteral("The stream has no video")},
        {QStringLiteral("В раскладке 1 и в развёрнутой ячейке всегда основной поток"), QStringLiteral("The 1×1 layout and a maximized cell always use the main stream")},
        {QStringLiteral("Введите пароль для входа в программу."), QStringLiteral("Enter the password to open the program.")},
        {QStringLiteral("Вести непрерывную запись в архив"), QStringLiteral("Record continuously to the archive")},
        {QStringLiteral("Во весь экран"), QStringLiteral("Full screen")},
        {QStringLiteral("Войти"), QStringLiteral("Sign in")},
        {QStringLiteral("Время"), QStringLiteral("Time")},
        {QStringLiteral("Выбрать все"), QStringLiteral("Select all")},
        {QStringLiteral("Выбрать поток…"), QStringLiteral("Choose stream…")},
        {QStringLiteral("Выход"), QStringLiteral("Exit")},
        {QStringLiteral("Готово к добавлению"), QStringLiteral("Ready to add")},
        {QStringLiteral("Группа"), QStringLiteral("Group")},
        {QStringLiteral("Группа:"), QStringLiteral("Group:")},
        {QStringLiteral("Переименовать группу…"), QStringLiteral("Rename group…")},
        {QStringLiteral("Переименовать группу"), QStringLiteral("Rename group")},
        {QStringLiteral("Новое название группы «%1»:"), QStringLiteral("New name for group “%1”:")},
        {QStringLiteral("Переименовать камеру…"), QStringLiteral("Rename camera…")},
        {QStringLiteral("Переименовать камеру"), QStringLiteral("Rename camera")},
        {QStringLiteral("Новое название камеры:"), QStringLiteral("New camera name:")},
        {QStringLiteral("Удалить эту камеру / канал регистратора…"), QStringLiteral("Delete this camera / recorder channel…")},
        {QStringLiteral("Удаление устройства"), QStringLiteral("Delete device")},
        {QStringLiteral("Удалить «%1» из PSVMS?\nЗапись будет остановлена, устройство исчезнет из списка. Уже сохранённые файлы архива останутся на диске."), QStringLiteral("Remove “%1” from PSVMS?\nRecording will stop and the device will be removed from the list. Existing archive files will remain on disk.")},
        {QStringLiteral("Диапазон IP:"), QStringLiteral("IP range:")},
        {QStringLiteral("Для этого адреса шаблон потоков неизвестен: введите адрес вручную"), QStringLiteral("No stream template is known for this address: enter the address manually")},
        {QStringLiteral("Добавить"), QStringLiteral("Add")},
        {QStringLiteral("Добавить Hikvision / HiWatch"), QStringLiteral("Add Hikvision / HiWatch")},
        {QStringLiteral("Добавить Hikvision / HiWatch…"), QStringLiteral("Add Hikvision / HiWatch…")},
        {QStringLiteral("Добавить iCSee…"), QStringLiteral("Add iCSee…")},
        {QStringLiteral("Добавить выбранные"), QStringLiteral("Add selected")},
        {QStringLiteral("Добавить камеру"), QStringLiteral("Add camera")},
        {QStringLiteral("Добавить камеры iCSee"), QStringLiteral("Add iCSee cameras")},
        {QStringLiteral("Добавлена"), QStringLiteral("Added")},
        {QStringLiteral("Добавлено"), QStringLiteral("Added")},
        {QStringLiteral("Добавлено камер: %1, с ошибкой: %2%3"), QStringLiteral("Cameras added: %1, failed: %2%3")},
        {QStringLiteral("Добавлено камер: %1. Состояние записи смотрите на вкладке «Устройства» и в «Журнале»."), QStringLiteral("Cameras added: %1. Check the recording status on the “Devices” tab and in “Events”.")},
        {QStringLiteral("Добавлено каналов: %1 (устройств: %2), с ошибкой: %3%4"), QStringLiteral("Channels added: %1 (devices: %2), failed: %3%4")},
        {QStringLiteral("Закрыть"), QStringLiteral("Close")},
        {QStringLiteral("Закрыть все"), QStringLiteral("Close all")},
        {QStringLiteral("Закрыть камеру"), QStringLiteral("Close camera")},
        {QStringLiteral("Записи за день (двойной щелчок — воспроизвести):"), QStringLiteral("Recordings for the day (double-click to play):")},
        {QStringLiteral("Запись выключена"), QStringLiteral("Recording off")},
        {QStringLiteral("Запись идёт"), QStringLiteral("Recording")},
        {QStringLiteral("Запрос…"), QStringLiteral("Requesting…")},
        {QStringLiteral("Зум"), QStringLiteral("Zoom")},
        {QStringLiteral("Изменить"), QStringLiteral("Edit")},
        {QStringLiteral("Изменить камеру"), QStringLiteral("Edit camera")},
        {QStringLiteral("Импорт ONVIF…"), QStringLiteral("Import ONVIF…")},
        {QStringLiteral("Импорт камер по ONVIF"), QStringLiteral("Import cameras via ONVIF")},
        {QStringLiteral("Имя / модель"), QStringLiteral("Name / model")},
        {QStringLiteral("КАМЕРЫ"), QStringLiteral("CAMERAS")},
        {QStringLiteral("Как у записи (без подпотока)"), QStringLiteral("Same as recording (no substream)")},
        {QStringLiteral("Камер: %1    Ведётся запись: %2    Архив: %3"), QStringLiteral("Cameras: %1    Recording: %2    Archive: %3")},
        {QStringLiteral("Камера"), QStringLiteral("Camera")},
        {QStringLiteral("Камера не вернула RTSP-адрес"), QStringLiteral("The camera did not return an RTSP address")},
        {QStringLiteral("Камера не вернула адрес media-сервиса (проверьте логин и пароль)"), QStringLiteral("The camera did not return a media service address (check the login and password)")},
        {QStringLiteral("Камера не отвечает (таймаут)"), QStringLiteral("The camera is not responding (timeout)")},
        {QStringLiteral("Камеры должны быть в вашей сети (по IP). Облачное подключение по серийному номеру не используется."), QStringLiteral("Cameras must be on your network (by IP). Cloud connection by serial number is not used.")},
        {QStringLiteral("Канал %1"), QStringLiteral("Channel %1")},
        {QStringLiteral("Каналов в регистраторе:"), QStringLiteral("Recorder channels:")},
        {QStringLiteral("Логин:"), QStringLiteral("Login:")},
        {QStringLiteral("Модель / тип"), QStringLiteral("Model / type")},
        {QStringLiteral("Мониторинг"), QStringLiteral("Monitoring")},
        {QStringLiteral("Нажмите «Найти в своей сети» или задайте диапазон IP."), QStringLiteral("Click “Find on my network” or set an IP range.")},
        {QStringLiteral("Название"), QStringLiteral("Name")},
        {QStringLiteral("Найдена"), QStringLiteral("Found")},
        {QStringLiteral("Найдено"), QStringLiteral("Found")},
        {QStringLiteral("Найдите устройства, введите логин и пароль и нажмите «Добавить выбранные»."), QStringLiteral("Find devices, enter the login and password, then click “Add selected”.")},
        {QStringLiteral("Найти в своей сети"), QStringLiteral("Find on my network")},
        {QStringLiteral("Найти в сети (SADP)"), QStringLiteral("Find on network (SADP)")},
        {QStringLiteral("Найти камеры"), QStringLiteral("Find cameras")},
        {QStringLiteral("Не отвечает (таймаут)"), QStringLiteral("Not responding (timeout)")},
        {QStringLiteral("Не удалось открыть БД: %1"), QStringLiteral("Could not open the database: %1")},
        {QStringLiteral("Не удалось подключиться: %1"), QStringLiteral("Could not connect: %1")},
        {QStringLiteral("Не удалось получить каналы регистратора: укажите их число вручную"), QStringLiteral("Could not get the recorder channels: enter their number manually")},
        {QStringLiteral("Не удалось получить параметры потока: %1"), QStringLiteral("Could not get stream parameters: %1")},
        {QStringLiteral("Не удалось создать сегмент: %1"), QStringLiteral("Could not create a segment: %1")},
        {QStringLiteral("Не удалось сохранить пароль."), QStringLiteral("Could not save the password.")},
        {QStringLiteral("Не хватает памяти"), QStringLiteral("Out of memory")},
        {QStringLiteral("Неверный логин или пароль"), QStringLiteral("Wrong login or password")},
        {QStringLiteral("Неверный логин или пароль (или устройство не активировано)"), QStringLiteral("Wrong login or password (or the device is not activated)")},
        {QStringLiteral("Неверный пароль."), QStringLiteral("Wrong password.")},
        {QStringLiteral("Неверный пароль. Вход временно заблокирован."), QStringLiteral("Wrong password. Sign-in is temporarily blocked.")},
        {QStringLiteral("Нет связи, переподключение…"), QStringLiteral("No connection, reconnecting…")},
        {QStringLiteral("Новые пароли не совпадают."), QStringLiteral("The new passwords do not match.")},
        {QStringLiteral("Новый пароль"), QStringLiteral("New password")},
        {QStringLiteral("Новый пароль слишком короткий: минимум %1 символов."), QStringLiteral("The new password is too short: at least %1 characters.")},
        {QStringLiteral("ОК"), QStringLiteral("OK")},
        {QStringLiteral("Обновить"), QStringLiteral("Refresh")},
        {QStringLiteral("Опрос камер: %1 из %2"), QStringLiteral("Querying cameras: %1 of %2")},
        {QStringLiteral("Опрос устройств: %1 из %2"), QStringLiteral("Querying devices: %1 of %2")},
        {QStringLiteral("Основной поток"), QStringLiteral("Main stream")},
        {QStringLiteral("Остановлено"), QStringLiteral("Stopped")},
        {QStringLiteral("Ответ не похож на Hikvision/HiWatch"), QStringLiteral("The response does not look like Hikvision/HiWatch")},
        {QStringLiteral("Отмена"), QStringLiteral("Cancel")},
        {QStringLiteral("Отметьте камеры для добавления."), QStringLiteral("Select the cameras to add.")},
        {QStringLiteral("Отметьте устройства для добавления."), QStringLiteral("Select the devices to add.")},
        {QStringLiteral("Очистить"), QStringLiteral("Clear")},
        {QStringLiteral("Ошибка записи (диск?): %1"), QStringLiteral("Write error (disk?): %1")},
        {QStringLiteral("Ошибка индекса: %1"), QStringLiteral("Index error: %1")},
        {QStringLiteral("Пароли не совпадают."), QStringLiteral("Passwords do not match.")},
        {QStringLiteral("Пароль"), QStringLiteral("Password")},
        {QStringLiteral("Пароль изменён."), QStringLiteral("Password changed.")},
        {QStringLiteral("Пароль слишком короткий: минимум %1 символов."), QStringLiteral("The password is too short: at least %1 characters.")},
        {QStringLiteral("Пароль:"), QStringLiteral("Password:")},
        {QStringLiteral("Первый запуск. Придумайте пароль для входа в программу (не короче 8 символов). Восстановить пароль невозможно: храните его в надёжном месте."), QStringLiteral("First launch. Create a password to sign in to the program (at least 8 characters). It cannot be recovered, so keep it somewhere safe.")},
        {QStringLiteral("Перетащите камеру сюда\nили дважды щёлкните её в списке"), QStringLiteral("Drag a camera here\nor double-click it in the list")},
        {QStringLiteral("Повторите новый"), QStringLiteral("Repeat new")},
        {QStringLiteral("Повторите пароль"), QStringLiteral("Repeat password")},
        {QStringLiteral("Подключение…"), QStringLiteral("Connecting…")},
        {QStringLiteral("Подпоток"), QStringLiteral("Substream")},
        {QStringLiteral("Подпоток для мозаики"), QStringLiteral("Substream for the grid")},
        {QStringLiteral("Поиск завершён. Найдено адресов: %1"), QStringLiteral("Search finished. Addresses found: %1")},
        {QStringLiteral("Поиск завершён. Найдено устройств: %1"), QStringLiteral("Search finished. Devices found: %1")},
        {QStringLiteral("Поиск завершён. Найдено устройств: %1. Если список пуст, используйте сканирование диапазона."), QStringLiteral("Search finished. Devices found: %1. If the list is empty, scan an IP range.")},
        {QStringLiteral("Поиск камер в сети…"), QStringLiteral("Looking for cameras on the network…")},
        {QStringLiteral("Поиск камеры…"), QStringLiteral("Search camera…")},
        {QStringLiteral("Поиск устройств в сети…"), QStringLiteral("Looking for devices on the network…")},
        {QStringLiteral("Показать или скрыть список камер"), QStringLiteral("Show or hide the camera list")},
        {QStringLiteral("Показывать основной поток вместо подпотока: выше качество, больше нагрузка"), QStringLiteral("Show the main stream instead of the substream: higher quality, more load")},
        {QStringLiteral("Получен 1 поток"), QStringLiteral("1 stream received")},
        {QStringLiteral("Получены основной поток и подпоток"), QStringLiteral("Main stream and substream received")},
        {QStringLiteral("Порт ONVIF: обычно 80; у камер Xiongmai/iCSee — 8899"), QStringLiteral("ONVIF port: usually 80; 8899 for Xiongmai/iCSee cameras")},
        {QStringLiteral("Порт закрыт"), QStringLiteral("Port closed")},
        {QStringLiteral("Поток для мозаики"), QStringLiteral("Stream for the grid")},
        {QStringLiteral("Поток записи"), QStringLiteral("Recording stream")},
        {QStringLiteral("Поток прерван: %1"), QStringLiteral("Stream interrupted: %1")},
        {QStringLiteral("Проверено %1 из %2, найдено: %3"), QStringLiteral("Checked %1 of %2, found: %3")},
        {QStringLiteral("Раскладка экрана"), QStringLiteral("Screen layout")},
        {QStringLiteral("Свернуть или развернуть меню"), QStringLiteral("Collapse or expand the menu")},
        {QStringLiteral("Сканирование завершено. Найдено адресов: %1"), QStringLiteral("Scan finished. Addresses found: %1")},
        {QStringLiteral("Сканирование завершено. Найдено устройств: %1"), QStringLiteral("Scan finished. Devices found: %1")},
        {QStringLiteral("Сканирование: %1 из %2, найдено: %3"), QStringLiteral("Scanning: %1 of %2, found: %3")},
        {QStringLiteral("Сканировать диапазон"), QStringLiteral("Scan range")},
        {QStringLiteral("Скорость"), QStringLiteral("Speed")},
        {QStringLiteral("Слишком большой диапазон: максимум %1 адресов за раз."), QStringLiteral("The range is too large: at most %1 addresses at a time.")},
        {QStringLiteral("Слишком много попыток. Подождите %1 с."), QStringLiteral("Too many attempts. Wait %1 s.")},
        {QStringLiteral("Смена пароля"), QStringLiteral("Change password")},
        {QStringLiteral("Сменить пароль…"), QStringLiteral("Change password…")},
        {QStringLiteral("Снять выбор"), QStringLiteral("Clear selection")},
        {QStringLiteral("Событие"), QStringLiteral("Event")},
        {QStringLiteral("События"), QStringLiteral("Events")},
        {QStringLiteral("Создать"), QStringLiteral("Create")},
        {QStringLiteral("Состояние"), QStringLiteral("Status")},
        {QStringLiteral("Текст"), QStringLiteral("Text")},
        {QStringLiteral("Текущий пароль"), QStringLiteral("Current password")},
        {QStringLiteral("Текущий пароль указан неверно."), QStringLiteral("The current password is wrong.")},
        {QStringLiteral("Текущий подпоток"), QStringLiteral("Current substream")},
        {QStringLiteral("Третий поток"), QStringLiteral("Third stream")},
        {QStringLiteral("У камеры нет видеопрофилей"), QStringLiteral("The camera has no video profiles")},
        {QStringLiteral("Удаление камеры"), QStringLiteral("Delete camera")},
        {QStringLiteral("Удалить"), QStringLiteral("Delete")},
        {QStringLiteral("Удалить камеру «%1»?\nУже записанные файлы архива останутся на диске."), QStringLiteral("Delete the camera “%1”?\nFiles already recorded to the archive will stay on disk.")},
        {QStringLiteral("Уже добавлена"), QStringLiteral("Already added")},
        {QStringLiteral("Уже добавлено"), QStringLiteral("Already added")},
        {QStringLiteral("Укажите корректный диапазон IPv4-адресов."), QStringLiteral("Enter a valid IPv4 address range.")},
        {QStringLiteral("Укажите логин камер."), QStringLiteral("Enter the camera login.")},
        {QStringLiteral("Укажите логин."), QStringLiteral("Enter the login.")},
        {QStringLiteral("Укажите название и корректный адрес потока, например rtsp://…"), QStringLiteral("Enter a name and a valid stream address, for example rtsp://…")},
        {QStringLiteral("Управление по ONVIF будет добавлено позже"), QStringLiteral("ONVIF control will be added later")},
        {QStringLiteral("Устройства"), QStringLiteral("Devices")},
        {QStringLiteral("Число каналов регистратора, если автоопределение не сработало"), QStringLiteral("Recorder channel count, if auto-detection failed")},
        {QStringLiteral("Это не Hikvision/HiWatch (нет ISAPI)"), QStringLiteral("This is not Hikvision/HiWatch (no ISAPI)")},
        {QStringLiteral("Язык интерфейса"), QStringLiteral("Interface language")},
        {QStringLiteral("Язык: "), QStringLiteral("Language: ")},
        {QStringLiteral("авто"), QStringLiteral("auto")},
        {QStringLiteral("для регистратора — по умолчанию его имя"), QStringLiteral("for a recorder, its name by default")},
        {QStringLiteral("идёт запись"), QStringLiteral("recording")},
        {QStringLiteral("камера"), QStringLiteral("camera")},
        {QStringLiteral("например, Двор"), QStringLiteral("for example, Yard")},
        {QStringLiteral("например, Склад"), QStringLiteral("for example, Warehouse")},
        {QStringLiteral("необязательно: поток меньшего разрешения для мозаики"), QStringLiteral("optional: a lower-resolution stream for the grid")},
        {QStringLiteral("порт"), QStringLiteral("port")},
        {QStringLiteral("регистратор"), QStringLiteral("recorder")},
        {QStringLiteral("у новых камер iCSee часто пустой"), QStringLiteral("new iCSee cameras often have an empty one")},
        {QStringLiteral("☰ Камеры"), QStringLiteral("☰ Cameras")},
        {QStringLiteral("2. Общие логин и пароль"), QStringLiteral("2. Shared login and password")},
        {QStringLiteral("Esc или двойной щелчок — выйти"), QStringLiteral("Esc or double-click to exit")},
        {QStringLiteral("Hik-Connect — облачный сервис Hikvision. Подключение к нему из сторонних программ возможно только через официальный SDK и ключи разработчика Hikvision (Open Platform), поэтому в этой версии получение камер из облака не работает."), QStringLiteral("Hik-Connect is Hikvision’s cloud service. Third-party programs can connect to it only through the official SDK and Hikvision Open Platform developer keys, so retrieving cameras from the cloud does not work in this version.")},
        {QStringLiteral("Hikvision / HiWatch по IP…"), QStringLiteral("Hikvision / HiWatch by IP…")},
        {QStringLiteral("IP-адрес"), QStringLiteral("IP address")},
        {QStringLiteral("e-mail или телефон"), QStringLiteral("e-mail or phone")},
        {QStringLiteral("Авто"), QStringLiteral("Auto")},
        {QStringLiteral("Авто (подпоток в мозаике)"), QStringLiteral("Auto (substream in the grid)")},
        {QStringLiteral("Авто: подпоток в мозаике, основной в раскладке 1 × 1"), QStringLiteral("Auto: substream in the grid, main stream in the 1 × 1 layout")},
        {QStringLiteral("Администратор"), QStringLiteral("Administrator")},
        {QStringLiteral("Администратор (все разделы)"), QStringLiteral("Administrator (all sections)")},
        {QStringLiteral("Азия"), QStringLiteral("Asia")},
        {QStringLiteral("Аккаунт"), QStringLiteral("Account")},
        {QStringLiteral("Америка"), QStringLiteral("Americas")},
        {QStringLiteral("Архив записей выбранной камеры"), QStringLiteral("Recording archive of the selected camera")},
        {QStringLiteral("Архив камеры…"), QStringLiteral("Camera archive…")},
        {QStringLiteral("Архив…"), QStringLiteral("Archive…")},
        {QStringLiteral("В резервную копию входят камеры, настройки программы, шаблоны и расположение камер в окне. Пользователи и пароли входа в копию не попадают.\n\nВнимание: адреса камер содержат их пароли. Храните файл копии в безопасном месте."), QStringLiteral("The backup includes cameras, program settings, templates and the camera layout in the window. Users and sign-in passwords are not included.\n\nWarning: camera addresses contain their passwords. Keep the backup file in a safe place.")},
        {QStringLiteral("В сети"), QStringLiteral("Online")},
        {QStringLiteral("Версия %1"), QStringLiteral("Version %1")},
        {QStringLiteral("Видео и папки"), QStringLiteral("Video and folders")},
        {QStringLiteral("Включать звук при показе"), QStringLiteral("Turn sound on when shown")},
        {QStringLiteral("Включить или выключить звук"), QStringLiteral("Turn sound on or off")},
        {QStringLiteral("Восстановить"), QStringLiteral("Restore")},
        {QStringLiteral("Восстановить из копии"), QStringLiteral("Restore from backup")},
        {QStringLiteral("Восстановить из копии…"), QStringLiteral("Restore from backup…")},
        {QStringLiteral("Восстановление"), QStringLiteral("Restore")},
        {QStringLiteral("Все производители"), QStringLiteral("All manufacturers")},
        {QStringLiteral("Всего камер"), QStringLiteral("Total cameras")},
        {QStringLiteral("Выберите папку"), QStringLiteral("Choose a folder")},
        {QStringLiteral("Выберите пользователя и введите пароль."), QStringLiteral("Choose a user and enter the password.")},
        {QStringLiteral("Выйти из учётной записи"), QStringLiteral("Sign out")},
        {QStringLiteral("Высокое (основной поток)"), QStringLiteral("High (main stream)")},
        {QStringLiteral("Высокое: всегда основной поток"), QStringLiteral("High: always the main stream")},
        {QStringLiteral("Дата и время на картинке"), QStringLiteral("Date and time on the picture")},
        {QStringLiteral("Для выделенных камер; если ничего не выделено — для всех"), QStringLiteral("For the selected cameras; if nothing is selected, for all")},
        {QStringLiteral("Добавить Hikvision / HiWatch по IP"), QStringLiteral("Add Hikvision / HiWatch by IP")},
        {QStringLiteral("Добавить новую камеру…"), QStringLiteral("Add a new camera…")},
        {QStringLiteral("Добавить пользователя…"), QStringLiteral("Add user…")},
        {QStringLiteral("Добавление отменено: нужен пароль."), QStringLiteral("Adding canceled: a password is required.")},
        {QStringLiteral("Добавлено каналов: %1 (%2). Способ определения: %3"), QStringLiteral("Channels added: %1 (%2). Detection method: %3")},
        {QStringLiteral("Добавлять камеры может только администратор"), QStringLiteral("Only an administrator can add cameras")},
        {QStringLiteral("Другой регион"), QStringLiteral("Other region")},
        {QStringLiteral("Европа"), QStringLiteral("Europe")},
        {QStringLiteral("Загрузка…"), QStringLiteral("Loading…")},
        {QStringLiteral("Записи ведутся сегментами по 5 минут. При смене папки новые записи пишутся в неё, старые остаются на прежнем месте. Скриншот делается из меню камеры (правая кнопка мыши)."), QStringLiteral("Recordings are made in 5-minute segments. When you change the folder, new recordings go to it and old ones stay where they were. Take a screenshot from the camera menu (right mouse button).")},
        {QStringLiteral("Запись"), QStringLiteral("Recording")},
        {QStringLiteral("Запомнить меня на этом компьютере"), QStringLiteral("Remember me on this computer")},
        {QStringLiteral("Запускать камеры автоматически при запуске программы"), QStringLiteral("Start cameras automatically when the program starts")},
        {QStringLiteral("Запустить запись"), QStringLiteral("Start recording")},
        {QStringLiteral("Запустить камеру"), QStringLiteral("Start camera")},
        {QStringLiteral("Заполнить всю ячейку (обрезать края)"), QStringLiteral("Fill the entire cell (crop edges)")},
        {QStringLiteral("Заполнить ячейки всей группы (обрезать края)"), QStringLiteral("Fill cells for the entire group (crop edges)")},
        {QStringLiteral("Применить заполнение ко всем камерам"), QStringLiteral("Apply fill mode to all cameras")},
        {QStringLiteral("Отключить заполнение у всех камер"), QStringLiteral("Disable fill mode for all cameras")},
        {QStringLiteral("Отключить заполнение для группы"), QStringLiteral("Disable fill mode for this group")},
        {QStringLiteral("Запустить показ"), QStringLiteral("Start viewing")},
        {QStringLiteral("Запустить показ камер в ячейках и запись"), QStringLiteral("Start showing cameras in the cells and recording")},
        {QStringLiteral("Звук"), QStringLiteral("Sound")},
        {QStringLiteral("Идёт запись"), QStringLiteral("Recording now")},
        {QStringLiteral("Импорт по ONVIF…"), QStringLiteral("Import via ONVIF…")},
        {QStringLiteral("Имя"), QStringLiteral("Name")},
        {QStringLiteral("Камер пока нет"), QStringLiteral("No cameras yet")},
        {QStringLiteral("Камеру вручную…"), QStringLiteral("Camera manually…")},
        {QStringLiteral("Камеры iCSee…"), QStringLiteral("iCSee cameras…")},
        {QStringLiteral("Каналов в регистраторе"), QStringLiteral("Recorder channels")},
        {QStringLiteral("Качество картинки"), QStringLiteral("Picture quality")},
        {QStringLiteral("Логин"), QStringLiteral("Login")},
        {QStringLiteral("Масштаб интерфейса (после перезапуска)"), QStringLiteral("Interface scale (after restart)")},
        {QStringLiteral("Модель / имя"), QStringLiteral("Model / name")},
        {QStringLiteral("На весь экран"), QStringLiteral("Full screen")},
        {QStringLiteral("Нажмите ▶, чтобы запустить"), QStringLiteral("Press ▶ to start")},
        {QStringLiteral("Название шаблона:"), QStringLiteral("Template name:")},
        {QStringLiteral("Настройки"), QStringLiteral("Settings")},
        {QStringLiteral("Настройки камеры"), QStringLiteral("Camera settings")},
        {QStringLiteral("Настройки камеры…"), QStringLiteral("Camera settings…")},
        {QStringLiteral("Не в сети"), QStringLiteral("Offline")},
        {QStringLiteral("Не запущена"), QStringLiteral("Not started")},
        {QStringLiteral("Не удалось добавить: %1"), QStringLiteral("Could not add: %1")},
        {QStringLiteral("Не удалось записать файл: %1"), QStringLiteral("Could not write the file: %1")},
        {QStringLiteral("Не удалось определить адрес потока камеры"), QStringLiteral("Could not determine the camera stream address")},
        {QStringLiteral("Не удалось прочитать файл: %1"), QStringLiteral("Could not read the file: %1")},
        {QStringLiteral("Не удалось сохранить снимок: %1"), QStringLiteral("Could not save the screenshot: %1")},
        {QStringLiteral("Нельзя удалить единственного пользователя."), QStringLiteral("The only user cannot be deleted.")},
        {QStringLiteral("Нельзя удалить пользователя, под которым выполнен вход."), QStringLiteral("You cannot delete the user you are signed in as.")},
        {QStringLiteral("Нельзя удалить последнего администратора."), QStringLiteral("The last administrator cannot be deleted.")},
        {QStringLiteral("Нет кадра для снимка"), QStringLiteral("No frame to capture")},
        {QStringLiteral("Нет видеопотока"), QStringLiteral("No video stream")},
        {QStringLiteral("Низкое (подпоток, экономия)"), QStringLiteral("Low (substream, saves resources)")},
        {QStringLiteral("Низкое: всегда подпоток (экономия ресурсов)"), QStringLiteral("Low: always the substream (saves resources)")},
        {QStringLiteral("Новый пароль для пользователя «%1». Оставьте пустым, чтобы убрать пароль."), QStringLiteral("New password for the user “%1”. Leave empty to remove the password.")},
        {QStringLiteral("Новый пользователь"), QStringLiteral("New user")},
        {QStringLiteral("Новый шаблон"), QStringLiteral("New template")},
        {QStringLiteral("Нужен пароль"), QStringLiteral("Password required")},
        {QStringLiteral("О программе"), QStringLiteral("About")},
        {QStringLiteral("Обзор…"), QStringLiteral("Browse…")},
        {QStringLiteral("Облако Hik-Connect"), QStringLiteral("Hik-Connect cloud")},
        {QStringLiteral("Общие"), QStringLiteral("General")},
        {QStringLiteral("Оператор"), QStringLiteral("Operator")},
        {QStringLiteral("Оператор (только просмотр)"), QStringLiteral("Operator (view only)")},
        {QStringLiteral("Оставьте новый пароль пустым, чтобы входить без пароля."), QStringLiteral("Leave the new password empty to sign in without a password.")},
        {QStringLiteral("Остановить запись"), QStringLiteral("Stop recording")},
        {QStringLiteral("Остановить показ"), QStringLiteral("Stop viewing")},
        {QStringLiteral("Остановить показ в ячейках (запись продолжается)"), QStringLiteral("Stop viewing in the cells (recording continues)")},
        {QStringLiteral("Открывать программу на весь экран"), QStringLiteral("Open the program in full screen")},
        {QStringLiteral("Открыть"), QStringLiteral("Open")},
        {QStringLiteral("Папка для записей видео"), QStringLiteral("Video recordings folder")},
        {QStringLiteral("Папка для скриншотов"), QStringLiteral("Screenshots folder")},
        {QStringLiteral("Пароль не обязателен: оставьте поля пустыми, чтобы пользователь входил без пароля."), QStringLiteral("A password is optional: leave the fields empty so the user signs in without a password.")},
        {QStringLiteral("Пароль пользователя"), QStringLiteral("User password")},
        {QStringLiteral("Пароль слишком короткий (минимум %1)."), QStringLiteral("The password is too short (at least %1).")},
        {QStringLiteral("Пароль…"), QStringLiteral("Password…")},
        {QStringLiteral("Перетащите заголовок камеры на другую ячейку: камеры поменяются местами; положение сохранится автоматически"), QStringLiteral("Drag by the title to swap cameras")},
        {QStringLiteral("Перетащите камеру сюда\nили нажмите «+»"), QStringLiteral("Drag a camera here\nor press “+”")},
        {QStringLiteral("Перетащите на другую ячейку"), QStringLiteral("Drop onto another cell")},
        {QStringLiteral("Подключиться"), QStringLiteral("Connect")},
        {QStringLiteral("Поиск в сети: Hikvision, HiWatch, Polyvision…"), QStringLiteral("Network search: Hikvision, HiWatch, Polyvision…")},
        {QStringLiteral("Поиск камер в сети: Hikvision, HiWatch, Polyvision"), QStringLiteral("Camera search on the network: Hikvision, HiWatch, Polyvision")},
        {QStringLiteral("Показывать дату и время на камерах"), QStringLiteral("Show date and time on cameras")},
        {QStringLiteral("Показывать дату и время на картинке"), QStringLiteral("Show date and time on the picture")},
        {QStringLiteral("Пользователи"), QStringLiteral("Users")},
        {QStringLiteral("Пользователи выбираются при входе. Администратор видит все разделы, оператор — только «Мониторинг» и «События». Пароль не обязателен."), QStringLiteral("Users are chosen at sign-in. An administrator sees all sections, an operator only “Monitoring” and “Events”. A password is optional.")},
        {QStringLiteral("Пользователь"), QStringLiteral("User")},
        {QStringLiteral("Пользователь с таким именем уже есть."), QStringLiteral("A user with this name already exists.")},
        {QStringLiteral("Порты"), QStringLiteral("Ports")},
        {QStringLiteral("Сеть и порты"), QStringLiteral("Network and ports")},
        {QStringLiteral("ИИ-помощник"), QStringLiteral("AI assistant")},
        {QStringLiteral("Включить ИИ-помощника Yandex в программе"), QStringLiteral("Enable Yandex AI assistant in the application")},
        {QStringLiteral("Открыть ИИ-чат"), QStringLiteral("Open AI chat")},
        {QStringLiteral("Сохранить параметры"), QStringLiteral("Save settings")},
        {QStringLiteral("Проверить порт"), QStringLiteral("Test port")},
        {QStringLiteral("Подставить порт службы"), QStringLiteral("Use service port")},
        {QStringLiteral("Порт доступен"), QStringLiteral("PORT IS REACHABLE")},
        {QStringLiteral("Порт недоступен"), QStringLiteral("PORT IS UNREACHABLE")},
        {QStringLiteral("RTSP — видеопоток"), QStringLiteral("RTSP — video stream")},
        {QStringLiteral("HTTP — веб-интерфейс"), QStringLiteral("HTTP — web interface")},
        {QStringLiteral("HTTPS — защищённый веб-интерфейс"), QStringLiteral("HTTPS — secure web interface")},
        {QStringLiteral("Пользовательский порт"), QStringLiteral("Custom port")},
        {QStringLiteral("Предел кадров в секунду на ячейку"), QStringLiteral("Frame rate limit per cell")},
        {QStringLiteral("Применён шаблон «%1»"), QStringLiteral("Template “%1” applied")},
        {QStringLiteral("Проверить и добавить"), QStringLiteral("Check and add")},
        {QStringLiteral("Проверить сейчас"), QStringLiteral("Check now")},
        {QStringLiteral("Проверка устройства %1…"), QStringLiteral("Checking device %1…")},
        {QStringLiteral("Производитель"), QStringLiteral("Manufacturer")},
        {QStringLiteral("Производительность"), QStringLiteral("Performance")},
        {QStringLiteral("Пропустить"), QStringLiteral("Skip")},
        {QStringLiteral("Пропущено: нужен пароль"), QStringLiteral("Skipped: password required")},
        {QStringLiteral("Работает с новыми и старыми моделями камер и регистраторов Hikvision и HiWatch. Если устройство потребует другой логин или пароль, программа спросит."), QStringLiteral("Works with new and old models of Hikvision and HiWatch cameras and recorders. If the device needs a different login or password, the program will ask.")},
        {QStringLiteral("Размер окна"), QStringLiteral("Window size")},
        {QStringLiteral("Расположение камер сохраняется автоматически. Здесь можно сохранить именованные шаблоны."), QStringLiteral("The camera layout is saved automatically. Here you can save named templates.")},
        {QStringLiteral("Регион"), QStringLiteral("Region")},
        {QStringLiteral("Анимация интерфейса (заставка, Demo, значки и переходы)"), QStringLiteral("Interface animations (splash, Demo, icons and transitions)")},
        {QStringLiteral("Режим для слабого компьютера"), QStringLiteral("Low-power mode")},
        {QStringLiteral("Режим для слабого компьютера: не более 8 кадров в секунду, без сглаживания и анимаций, реже проверяется связь с камерами. Ещё сильнее нагрузку снижает показ подпотока (у камеры: «Качество картинки → Низкое») и меньшее число ячеек в раскладке."), QStringLiteral("Low-power mode: no more than 8 frames per second, no smoothing or animations, camera connectivity is checked less often. The load is reduced even more by showing the substream (camera: “Picture quality → Low”) and fewer cells in the layout.")},
        {QStringLiteral("Резервная копия"), QStringLiteral("Backup")},
        {QStringLiteral("Резервная копия сохранена:\n%1"), QStringLiteral("Backup saved:\n%1")},
        {QStringLiteral("Роль"), QStringLiteral("Role")},
        {QStringLiteral("Россия и СНГ"), QStringLiteral("Russia and CIS")},
        {QStringLiteral("Сбросить масштаб"), QStringLiteral("Reset zoom")},
        {QStringLiteral("Связь"), QStringLiteral("Connection")},
        {QStringLiteral("Сглаживание картинки"), QStringLiteral("Picture smoothing")},
        {QStringLiteral("Система видеонаблюдения"), QStringLiteral("Video surveillance system")},
        {QStringLiteral("Снимок сохранён: %1"), QStringLiteral("Screenshot saved: %1")},
        {QStringLiteral("Снимок экрана"), QStringLiteral("Screenshot")},
        {QStringLiteral("Создать резервную копию…"), QStringLiteral("Create backup…")},
        {QStringLiteral("Состояние камер"), QStringLiteral("Camera status")},
        {QStringLiteral("Показывать счётчики «В сети» и «Не в сети» и кнопку «Состояние камер»"), QStringLiteral("Show the Online and Offline counters and the Camera status button")},
        {QStringLiteral("Открыть подробное состояние всех камер"), QStringLiteral("Open detailed status for all cameras")},
        {QStringLiteral("Сохранить текущее расположение как шаблон…"), QStringLiteral("Save the current layout as a template…")},
        {QStringLiteral("Статус: не подключено"), QStringLiteral("Status: not connected")},
        {QStringLiteral("Статус: недоступно в этой версии (нужны ключи Hikvision Open Platform). Добавляйте камеры по IP в локальной сети."), QStringLiteral("Status: unavailable in this version (Hikvision Open Platform keys are required). Add cameras by IP on the local network.")},
        {QStringLiteral("Текущие камеры и настройки будут заменены данными из копии, программа перезапустится. Продолжить?"), QStringLiteral("The current cameras and settings will be replaced with the data from the backup, and the program will restart. Continue?")},
        {QStringLiteral("Удаление пользователя"), QStringLiteral("Delete user")},
        {QStringLiteral("Удалить выбранные камеры (%1)?\nУже записанные файлы архива останутся на диске."), QStringLiteral("Delete the selected cameras (%1)?\nFiles already recorded to the archive will stay on disk.")},
        {QStringLiteral("Удалить выбранное"), QStringLiteral("Delete selected")},
        {QStringLiteral("Удалить все камеры"), QStringLiteral("Delete all cameras")},
        {QStringLiteral("Удалить ВСЕ камеры и каналы регистраторов (%1) из PSVMS?\nВсе потоки и запись будут остановлены. Уже сохранённые файлы архива останутся на диске. Это действие нельзя отменить."), QStringLiteral("Delete ALL cameras and recorder channels (%1) from PSVMS?\nAll streams and recording will stop. Existing archive files will remain on disk. This action cannot be undone.")},
        {QStringLiteral("Удалить все"), QStringLiteral("Delete all")},
        {QStringLiteral("Удалить все камеры и каналы регистраторов из списка PSVMS"), QStringLiteral("Remove all cameras and recorder channels from the PSVMS list")},
        {QStringLiteral("Удалить пользователя «%1»?"), QStringLiteral("Delete the user “%1”?")},
        {QStringLiteral("Удалить шаблон"), QStringLiteral("Delete template")},
        {QStringLiteral("Укажите IP-адрес устройства."), QStringLiteral("Enter the device IP address.")},
        {QStringLiteral("Укажите имя пользователя."), QStringLiteral("Enter a user name.")},
        {QStringLiteral("Устройство %1 требует логин и пароль.\nВведите данные для входа."), QStringLiteral("Device %1 requires a login and password.\nEnter the sign-in details.")},
        {QStringLiteral("Файл повреждён или не является резервной копией программы."), QStringLiteral("The file is damaged or is not a backup of this program.")},
        {QStringLiteral("Формат даты и времени"), QStringLiteral("Date and time format")},
        {QStringLiteral("Шаблон «%1» сохранён"), QStringLiteral("Template “%1” saved")},
        {QStringLiteral("Шаблоны"), QStringLiteral("Templates")},
        {QStringLiteral("Язык"), QStringLiteral("Language")},
        {QStringLiteral("Яркость"), QStringLiteral("Brightness")},
        {QStringLiteral("администратор"), QStringLiteral("administrator")},
        {QStringLiteral("без пароля"), QStringLiteral("no password")},
        {QStringLiteral("вход"), QStringLiteral("sign in")},
        {QStringLiteral("задан"), QStringLiteral("set")},
        {QStringLiteral("необязательно"), QStringLiteral("optional")},
        {QStringLiteral("оператор"), QStringLiteral("operator")},
        {QStringLiteral("⏹ Остановить показ"), QStringLiteral("⏹ Stop viewing")},
        {QStringLiteral("▶ Запустить"), QStringLiteral("▶ Start")},
        {QStringLiteral("● В сети: %1"), QStringLiteral("● Online: %1")},
        {QStringLiteral("● Не в сети: %1"), QStringLiteral("● Offline: %1")},
    };
    return dict;
}

} // namespace

void I18n::init()
{
    const QString saved = QSettings().value(QStringLiteral("ui/lang")).toString();
    if (saved == QLatin1String("en"))
        g_lang = Lang::En;
    else if (saved == QLatin1String("ru"))
        g_lang = Lang::Ru;
    else
        g_lang = (QLocale::system().language() == QLocale::Russian) ? Lang::Ru : Lang::En;
    QLocale::setDefault(QLocale(g_lang == Lang::Ru ? QLocale::Russian : QLocale::English));
}

I18n::Lang I18n::language()
{
    return g_lang;
}

QString I18n::code()
{
    return g_lang == Lang::Ru ? QStringLiteral("ru") : QStringLiteral("en");
}

void I18n::setLanguage(Lang lang)
{
    g_lang = lang;
    QSettings().setValue(QStringLiteral("ui/lang"), code());
    QLocale::setDefault(QLocale(lang == Lang::Ru ? QLocale::Russian : QLocale::English));
}

void I18n::setLanguageByCode(const QString &c)
{
    setLanguage(c == QLatin1String("en") ? Lang::En : Lang::Ru);
}

void I18n::requestRestart()
{
    g_restartRequested = true;
    QCoreApplication::quit();
}

bool I18n::takeRestartRequest()
{
    const bool r = g_restartRequested;
    g_restartRequested = false;
    return r;
}

QString I18n::translate(const char *ru)
{
    const QString key = QString::fromUtf8(ru);
    if (g_lang == Lang::En) {
        const auto it = englishDict().constFind(key);
        if (it != englishDict().constEnd())
            return it.value();
    }
    return key;
}

// ======================================================================
// core/onvif.cpp
// ======================================================================
namespace {

constexpr int kTimeoutMs = 6000;

bool isName_onvif(const QXmlStreamReader &x, const char *n)
{
    return x.name() == QLatin1String(n);
}

// ---------- SOAP ----------

// Конверт SOAP 1.2; при заданном логине добавляется WS-Security UsernameToken (PasswordDigest).
// offsetSecs — разница часов камеры и компьютера: без неё многие камеры отклоняют подпись.
QByteArray soapEnvelope(const QByteArray &body, const QString &user = QString(),
                        const QString &pass = QString(), qint64 offsetSecs = 0)
{
    QByteArray header;
    if (!user.isEmpty()) {
        QByteArray nonce(16, '\0');
        for (char &c : nonce)
            c = char(QRandomGenerator::global()->bounded(256));
        const QByteArray created =
            QDateTime::currentDateTimeUtc().addSecs(offsetSecs).toString(Qt::ISODate).toUtf8();
        const QByteArray digest =
            QCryptographicHash::hash(nonce + created + pass.toUtf8(), QCryptographicHash::Sha1).toBase64();

        header = QByteArray(
            "<s:Header><wsse:Security s:mustUnderstand=\"1\" "
            "xmlns:wsse=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\" "
            "xmlns:wsu=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\">"
            "<wsse:UsernameToken><wsse:Username>")
            + user.toHtmlEscaped().toUtf8()
            + QByteArray("</wsse:Username><wsse:Password Type=\"http://docs.oasis-open.org/wss/2004/01/"
                         "oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">")
            + digest
            + QByteArray("</wsse:Password><wsse:Nonce EncodingType=\"http://docs.oasis-open.org/wss/2004/01/"
                         "oasis-200401-wss-soap-message-security-1.0#Base64Binary\">")
            + nonce.toBase64()
            + QByteArray("</wsse:Nonce><wsu:Created>") + created
            + QByteArray("</wsu:Created></wsse:UsernameToken></wsse:Security></s:Header>");
    }

    return QByteArray(
               "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
               "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" "
               "xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\" "
               "xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\" "
               "xmlns:tt=\"http://www.onvif.org/ver10/schema\">")
           + header + QByteArray("<s:Body>") + body + QByteArray("</s:Body></s:Envelope>");
}

struct SoapReply {
    bool ok = false;
    QByteArray body;
    QString error;
};

QString faultText(const QByteArray &xml)
{
    QXmlStreamReader x(xml);
    while (!x.atEnd()) {
        x.readNext();
        if (x.isStartElement() && (isName_onvif(x, "Text") || isName_onvif(x, "faultstring")))
            return x.readElementText().trimmed();
    }
    return QString();
}

void postSoap(QNetworkAccessManager *nam, QObject *ctx, const QUrl &url, const QByteArray &body,
              int timeoutMs, std::function<void(const SoapReply &)> cb)
{
    if (url.scheme().compare(QLatin1String("https"), Qt::CaseInsensitive) != 0)
        SecurityAudit::write(QStringLiteral("onvif_plain_http_warning"), url.host());
    QNetworkRequest req(url);
    if (url.scheme().compare(QLatin1String("https"), Qt::CaseInsensitive) == 0) {
        QSslConfiguration ssl = QSslConfiguration::defaultConfiguration();
        ssl.setPeerVerifyMode(QSslSocket::VerifyPeer);
        req.setSslConfiguration(ssl);
    }
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/soap+xml; charset=utf-8"));
    req.setTransferTimeout(timeoutMs);
    QNetworkReply *reply = nam->post(req, body);

    QObject::connect(reply, &QNetworkReply::finished, ctx, [reply, cb] {
        SoapReply r;
        r.body = reply->readAll();
        r.ok = reply->error() == QNetworkReply::NoError;
        if (!r.ok) {
            const QString fault = faultText(r.body);
            if (reply->error() == QNetworkReply::AuthenticationRequiredError
                || fault.contains(QLatin1String("authorized"), Qt::CaseInsensitive))
                r.error = T("Неверный логин или пароль");
            else if (reply->error() == QNetworkReply::TimeoutError)
                r.error = T("Камера не отвечает (таймаут)");
            else
                r.error = fault.isEmpty() ? reply->errorString() : fault;
        }
        reply->deleteLater();
        cb(r);
    });
}

// ---------- разбор ответов ----------

QString firstText(const QByteArray &xml, const char *name)
{
    QXmlStreamReader x(xml);
    while (!x.atEnd()) {
        x.readNext();
        if (x.isStartElement() && isName_onvif(x, name))
            return x.readElementText().trimmed();
    }
    return QString();
}

// Разница «время камеры − наше UTC» в секундах
qint64 parseTimeOffset(const QByteArray &xml)
{
    QXmlStreamReader x(xml);
    bool inUtc = false;
    int h = -1, mi = -1, s = -1, y = -1, mo = -1, d = -1;
    while (!x.atEnd()) {
        x.readNext();
        if (x.isStartElement()) {
            if (isName_onvif(x, "UTCDateTime")) {
                inUtc = true;
            } else if (inUtc) {
                if (isName_onvif(x, "Hour")) h = x.readElementText().toInt();
                else if (isName_onvif(x, "Minute")) mi = x.readElementText().toInt();
                else if (isName_onvif(x, "Second")) s = x.readElementText().toInt();
                else if (isName_onvif(x, "Year")) y = x.readElementText().toInt();
                else if (isName_onvif(x, "Month")) mo = x.readElementText().toInt();
                else if (isName_onvif(x, "Day")) d = x.readElementText().toInt();
            }
        } else if (x.isEndElement() && isName_onvif(x, "UTCDateTime")) {
            break;
        }
    }
    if (h < 0 || mi < 0 || s < 0 || y < 0 || mo < 0 || d < 0)
        return 0;
    QDateTime cam(QDate(y, mo, d), QTime(h, mi, s));
    cam.setTimeSpec(Qt::UTC);
    if (!cam.isValid())
        return 0;
    return QDateTime::currentDateTimeUtc().secsTo(cam);
}

// Адрес сервиса по имени категории (Media, PTZ, ...) из ответа GetCapabilities
QString capabilityXAddr(const QByteArray &xml, const char *category)
{
    QXmlStreamReader x(xml);
    bool inCategory = false;
    while (!x.atEnd()) {
        x.readNext();
        if (x.isStartElement()) {
            if (isName_onvif(x, category))
                inCategory = true;
            else if (inCategory && isName_onvif(x, "XAddr"))
                return x.readElementText().trimmed();
        } else if (x.isEndElement() && isName_onvif(x, category)) {
            inCategory = false;
        }
    }
    return QString();
}

QString mediaXAddr(const QByteArray &xml)
{
    return capabilityXAddr(xml, "Media");
}

struct Profile {
    QString token;
    int w = 0;
    int h = 0;
    qint64 area() const { return qint64(w) * h; }
};

QList<Profile> parseProfiles(const QByteArray &xml)
{
    QList<Profile> out;
    QXmlStreamReader x(xml);
    bool inEnc = false, inRes = false;
    while (!x.atEnd()) {
        x.readNext();
        if (x.isStartElement()) {
            if (isName_onvif(x, "Profiles")) {
                Profile p;
                p.token = x.attributes().value(QLatin1String("token")).toString();
                out.append(p);
            } else if (isName_onvif(x, "VideoEncoderConfiguration")) {
                inEnc = true;
            } else if (inEnc && isName_onvif(x, "Resolution")) {
                inRes = true;
            } else if (inEnc && inRes && !out.isEmpty()) {
                if (isName_onvif(x, "Width")) out.last().w = x.readElementText().toInt();
                else if (isName_onvif(x, "Height")) out.last().h = x.readElementText().toInt();
            }
        } else if (x.isEndElement()) {
            if (isName_onvif(x, "Resolution")) inRes = false;
            else if (isName_onvif(x, "VideoEncoderConfiguration")) inEnc = false;
        }
    }
    return out;
}

QByteArray streamUriBody(const QString &token)
{
    return QByteArray("<trt:GetStreamUri><trt:StreamSetup><tt:Stream>RTP-Unicast</tt:Stream>"
                      "<tt:Transport><tt:Protocol>RTSP</tt:Protocol></tt:Transport></trt:StreamSetup>"
                      "<trt:ProfileToken>")
           + token.toHtmlEscaped().toUtf8() + QByteArray("</trt:ProfileToken></trt:GetStreamUri>");
}

QString withCredentials(const QString &uri, const OnvifRequest &req)
{
    QUrl u(uri);
    if (!u.isValid())
        return uri;
    if (u.userName().isEmpty() && !req.user.isEmpty()) {
        u.setUserName(req.user);
        u.setPassword(req.password);
    }
    return u.toString(QUrl::FullyEncoded);
}

OnvifDevice parseScopes(OnvifDevice dev, const QString &scopes)
{
    for (const QString &tok : scopes.split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
        int p = tok.indexOf(QLatin1String("/name/"));
        if (p >= 0) {
            dev.name = QUrl::fromPercentEncoding(tok.mid(p + 6).toUtf8());
            continue;
        }
        p = tok.indexOf(QLatin1String("/hardware/"));
        if (p >= 0)
            dev.hardware = QUrl::fromPercentEncoding(tok.mid(p + 10).toUtf8());
    }
    return dev;
}

} // namespace

// ===========================================================================
// OnvifDiscovery
// ===========================================================================

OnvifDiscovery::OnvifDiscovery(QObject *parent) : QObject(parent)
{
    m_socket = new QUdpSocket(this);
    connect(m_socket, &QUdpSocket::readyRead, this, &OnvifDiscovery::onReadyRead);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, [this] {
        m_running = false;
        emit finished();
    });
}

void OnvifDiscovery::start(int timeoutMs)
{
    stop();
    m_seen.clear();

    if (m_socket->state() != QAbstractSocket::BoundState
        && !m_socket->bind(QHostAddress::AnyIPv4, 0)) {
        emit finished();
        return;
    }
    m_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 4);

    m_running = true;
    sendProbes();
    QTimer::singleShot(1000, this, [this] { if (m_running) sendProbes(); });   // повтор для надёжности
    m_timer->start(timeoutMs);
}

void OnvifDiscovery::stop()
{
    m_running = false;
    m_timer->stop();
}

void OnvifDiscovery::sendProbes()
{
    const QByteArray probe =
        QByteArray(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<e:Envelope xmlns:e=\"http://www.w3.org/2003/05/soap-envelope\" "
            "xmlns:w=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
            "xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "
            "xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\"><e:Header>"
            "<w:MessageID>uuid:")
        + QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8()
        + QByteArray(
            "</w:MessageID><w:To e:mustUnderstand=\"true\">urn:schemas-xmlsoap-org:ws:2005:04:discovery</w:To>"
            "<w:Action e:mustUnderstand=\"true\">http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</w:Action>"
            "</e:Header><e:Body><d:Probe><d:Types>dn:NetworkVideoTransmitter</d:Types></d:Probe></e:Body>"
            "</e:Envelope>");

    const QHostAddress group(QStringLiteral("239.255.255.250"));
    for (const QNetworkInterface &ni : QNetworkInterface::allInterfaces()) {
        const auto fl = ni.flags();
        if (!(fl & QNetworkInterface::IsUp) || !(fl & QNetworkInterface::IsRunning)
            || (fl & QNetworkInterface::IsLoopBack) || !(fl & QNetworkInterface::CanMulticast))
            continue;
        m_socket->setMulticastInterface(ni);   // шлём через каждый сетевой адаптер
        m_socket->writeDatagram(probe, group, 3702);
    }
}

void OnvifDiscovery::onReadyRead()
{
    while (m_socket->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(int(m_socket->pendingDatagramSize()));
        m_socket->readDatagram(data.data(), data.size());
        if (!m_running)
            continue;

        QString xaddrs, scopes;
        QXmlStreamReader x(data);
        while (!x.atEnd()) {
            x.readNext();
            if (x.isStartElement()) {
                if (isName_onvif(x, "XAddrs")) xaddrs = x.readElementText();
                else if (isName_onvif(x, "Scopes")) scopes = x.readElementText();
            }
        }

        OnvifDevice dev;
        for (const QString &u : xaddrs.split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
            const QUrl url(u);
            if (url.scheme() == QLatin1String("http") && !url.host().contains(QLatin1Char(':'))) {
                dev.xaddr = u;
                dev.address = url.host();
                break;
            }
        }
        if (dev.xaddr.isEmpty() || m_seen.contains(dev.address))
            continue;
        m_seen.insert(dev.address);
        emit deviceFound(parseScopes(dev, scopes));
    }
}

// ===========================================================================
// OnvifScanner
// ===========================================================================

OnvifScanner::OnvifScanner(QObject *parent) : QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);
}

void OnvifScanner::start(quint32 firstIp, quint32 lastIp, quint16 port, int concurrency)
{
    stop();
    ++m_gen;
    m_next = firstIp;
    m_last = lastIp;
    m_port = port;
    m_conc = qMax(1, concurrency);
    m_total = int(m_last - m_next + 1);
    m_done = 0;
    m_active = 0;
    m_running = true;
    pump();
}

void OnvifScanner::stop()
{
    m_running = false;
    ++m_gen;
}

void OnvifScanner::pump()
{
    while (m_running && m_active < m_conc && m_next <= m_last) {
        const QString host = QHostAddress(quint32(m_next++)).toString();
        const QUrl url(QStringLiteral("http://%1:%2/onvif/device_service").arg(host).arg(m_port));
        const int gen = m_gen;
        ++m_active;

        postSoap(m_nam, this, url, soapEnvelope("<tds:GetSystemDateAndTime/>"), 1500,
                 [this, gen, host, url](const SoapReply &r) {
            if (gen != m_gen)
                return;
            --m_active;
            ++m_done;
            // даже SOAP-ошибка с HTTP 4xx означает, что по адресу отвечает ONVIF-устройство
            if (r.body.contains("Envelope")) {
                OnvifDevice d;
                d.address = host;
                d.xaddr = url.toString();
                emit deviceFound(d);
            }
            emit progress(m_done, m_total);
            pump();
        });
    }
    if (m_running && m_next > m_last && m_active == 0) {
        m_running = false;
        emit finished();
    }
}

// ===========================================================================
// OnvifResolver
// ===========================================================================

OnvifResolver::OnvifResolver(QObject *parent) : QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);
}

void OnvifResolver::start(const QList<OnvifRequest> &requests, int concurrency)
{
    ++m_gen;
    m_requests = requests;
    m_next = 0;
    m_done = 0;
    m_active = 0;
    m_conc = qMax(1, concurrency);
    m_running = true;
    if (requests.isEmpty()) {
        m_running = false;
        emit finished();
        return;
    }
    pump();
}

void OnvifResolver::cancel()
{
    m_running = false;
    ++m_gen;
}

void OnvifResolver::pump()
{
    while (m_running && m_active < m_conc && m_next < m_requests.size()) {
        ++m_active;
        runJob(m_next++);
    }
}

void OnvifResolver::jobDone(int idx, const OnvifStreams &s, int gen)
{
    if (gen != m_gen)
        return;
    --m_active;
    ++m_done;
    emit resolved(idx, s);
    emit progress(m_done, int(m_requests.size()));
    if (m_done >= m_requests.size()) {
        m_running = false;
        emit finished();
    } else {
        pump();
    }
}

void OnvifResolver::runJob(int idx)
{
    const OnvifRequest req = m_requests.at(idx);
    const int gen = m_gen;
    const QUrl devUrl(req.xaddr);

    auto fail = [this, idx, gen](const QString &msg) {
        OnvifStreams s;
        s.error = msg;
        jobDone(idx, s, gen);
    };

    // 1. Время камеры (без авторизации) — нужно для подписи WS-Security
    postSoap(m_nam, this, devUrl, soapEnvelope("<tds:GetSystemDateAndTime/>"), kTimeoutMs,
             [=](const SoapReply &r1) {
        if (gen != m_gen)
            return;
        const qint64 offset = r1.ok ? parseTimeOffset(r1.body) : 0;

        // 2. Адрес media-сервиса
        postSoap(m_nam, this, devUrl,
                 soapEnvelope("<tds:GetCapabilities><tds:Category>Media</tds:Category></tds:GetCapabilities>",
                              req.user, req.password, offset),
                 kTimeoutMs, [=](const SoapReply &r2) {
            if (gen != m_gen)
                return;
            if (!r2.ok) {
                fail(r2.error);
                return;
            }
            QUrl mediaUrl(mediaXAddr(r2.body));
            if (!mediaUrl.isValid() || mediaUrl.host().isEmpty()) {
                fail(T("Камера не вернула адрес media-сервиса (проверьте логин и пароль)"));
                return;
            }
            mediaUrl.setHost(devUrl.host());   // камеры за NAT иногда отдают внутренний адрес

            // 3. Профили видео
            postSoap(m_nam, this, mediaUrl,
                     soapEnvelope("<trt:GetProfiles/>", req.user, req.password, offset),
                     kTimeoutMs, [=](const SoapReply &r3) {
                if (gen != m_gen)
                    return;
                if (!r3.ok) {
                    fail(r3.error);
                    return;
                }
                QList<Profile> valid;
                const QList<Profile> all = parseProfiles(r3.body);
                for (const Profile &p : all)
                    if (!p.token.isEmpty() && p.area() > 0)
                        valid.append(p);
                if (valid.isEmpty() && !all.isEmpty() && !all.first().token.isEmpty())
                    valid.append(all.first());   // разрешение неизвестно — берём первый профиль
                if (valid.isEmpty()) {
                    fail(T("У камеры нет видеопрофилей"));
                    return;
                }

                const auto byArea = [](const Profile &a, const Profile &b) { return a.area() < b.area(); };
                const Profile mainP = *std::max_element(valid.begin(), valid.end(), byArea);
                Profile subP = *std::min_element(valid.begin(), valid.end(), byArea);
                if (subP.token == mainP.token)
                    subP.token.clear();   // профиль всего один

                // 4. RTSP-адрес основного потока
                postSoap(m_nam, this, mediaUrl,
                         soapEnvelope(streamUriBody(mainP.token), req.user, req.password, offset),
                         kTimeoutMs, [=](const SoapReply &r4) {
                    if (gen != m_gen)
                        return;
                    if (!r4.ok) {
                        fail(r4.error);
                        return;
                    }
                    const QString mainUri = firstText(r4.body, "Uri");
                    if (mainUri.isEmpty()) {
                        fail(T("Камера не вернула RTSP-адрес"));
                        return;
                    }
                    OnvifStreams s;
                    s.ok = true;
                    s.mainUrl = withCredentials(mainUri, req);

                    // 5. PTZ: есть ли у камеры поворотный механизм и адрес его сервиса
                    auto withPtz = [=](OnvifStreams out) {
                        postSoap(m_nam, this, devUrl,
                                 soapEnvelope("<tds:GetCapabilities><tds:Category>PTZ</tds:Category></tds:GetCapabilities>",
                                              req.user, req.password, offset),
                                 kTimeoutMs, [=](const SoapReply &r6) {
                            if (gen != m_gen)
                                return;
                            OnvifStreams final = out;
                            if (r6.ok) {
                                const QString ptz = capabilityXAddr(r6.body, "PTZ");
                                if (!ptz.isEmpty()) {
                                    final.ptzXAddr = ptz;
                                    final.ptzToken = mainP.token;
                                    final.hasPtz = true;
                                }
                            }
                            jobDone(idx, final, gen);
                        });
                    };

                    if (subP.token.isEmpty()) {
                        withPtz(s);
                        return;
                    }

                    // 6. RTSP-адрес дополнительного потока (для мозаики)
                    postSoap(m_nam, this, mediaUrl,
                             soapEnvelope(streamUriBody(subP.token), req.user, req.password, offset),
                             kTimeoutMs, [=](const SoapReply &r5) {
                        if (gen != m_gen)
                            return;
                        OnvifStreams out = s;
                        if (r5.ok) {
                            const QString subUri = firstText(r5.body, "Uri");
                            if (!subUri.isEmpty())
                                out.subUrl = withCredentials(subUri, req);
                        }
                        withPtz(out);   // без подпотока тоже успех
                    });
                });
            });
        });
    });
}

// ======================================================================
// core/onvif_ptz.cpp
// ======================================================================
namespace {

// Обычный (не digest) логин в заголовке PTZ-запроса — большинство камер его принимают;
// камеры со строгой WS-Security-аутентификацией для этого разговора пока не поддержаны.
QByteArray ptzEnvelope(const QString &body, const PtzTarget &t)
{
    QByteArray header;
    if (!t.user.isEmpty()) {
        header = QByteArray(
            "<s:Header><wsse:Security xmlns:wsse=\"http://docs.oasis-open.org/wss/2004/01/"
            "oasis-200401-wss-wssecurity-secext-1.0.xsd\"><wsse:UsernameToken><wsse:Username>")
            + t.user.toHtmlEscaped().toUtf8()
            + "</wsse:Username><wsse:Password>" + t.password.toHtmlEscaped().toUtf8()
            + "</wsse:Password></wsse:UsernameToken></wsse:Security></s:Header>";
    }
    return QByteArray(
               "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
               "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" "
               "xmlns:tptz=\"http://www.onvif.org/ver20/ptz/wsdl\" "
               "xmlns:tt=\"http://www.onvif.org/ver10/schema\">")
           + header + "<s:Body>" + body.toUtf8() + "</s:Body></s:Envelope>";
}

void send(QNetworkAccessManager *nam, const PtzTarget &t, const QString &body)
{
    const QUrl ptzUrl(t.xAddr);
    if (ptzUrl.scheme().compare(QLatin1String("https"), Qt::CaseInsensitive) != 0 && !t.password.isEmpty())
        SecurityAudit::write(QStringLiteral("ptz_plain_http_warning"), ptzUrl.host());
    QNetworkRequest req(ptzUrl);
    if (ptzUrl.scheme().compare(QLatin1String("https"), Qt::CaseInsensitive) == 0) {
        QSslConfiguration ssl = QSslConfiguration::defaultConfiguration();
        ssl.setPeerVerifyMode(QSslSocket::VerifyPeer);
        req.setSslConfiguration(ssl);
    }
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/soap+xml; charset=utf-8"));
    req.setTransferTimeout(3000);
    QNetworkReply *reply = nam->post(req, ptzEnvelope(body, t));
    QObject::connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

} // namespace

void ptzContinuousMove(QNetworkAccessManager *nam, const PtzTarget &target, double x, double y, double zoom)
{
    if (!target.valid())
        return;
    const QString body = QStringLiteral(
        "<tptz:ContinuousMove><tptz:ProfileToken>%1</tptz:ProfileToken><tptz:Velocity>"
        "<tt:PanTilt x=\"%2\" y=\"%3\" xmlns:tt=\"http://www.onvif.org/ver10/schema\"/>"
        "<tt:Zoom x=\"%4\" xmlns:tt=\"http://www.onvif.org/ver10/schema\"/>"
        "</tptz:Velocity></tptz:ContinuousMove>")
        .arg(target.token.toHtmlEscaped())
        .arg(x, 0, 'f', 2).arg(y, 0, 'f', 2).arg(zoom, 0, 'f', 2);
    send(nam, target, body);
}

void ptzStop(QNetworkAccessManager *nam, const PtzTarget &target)
{
    if (!target.valid())
        return;
    const QString body = QStringLiteral(
        "<tptz:Stop><tptz:ProfileToken>%1</tptz:ProfileToken>"
        "<tptz:PanTilt>true</tptz:PanTilt><tptz:Zoom>true</tptz:Zoom></tptz:Stop>")
        .arg(target.token.toHtmlEscaped());
    send(nam, target, body);
}

// ======================================================================
// core/portscan.cpp
// ======================================================================
PortScanner::PortScanner(QObject *parent) : QObject(parent) {}

void PortScanner::start(quint32 firstIp, quint32 lastIp, quint16 port, int concurrency, int timeoutMs)
{
    stop();
    ++m_gen;
    m_next = firstIp;
    m_last = lastIp;
    m_port = port;
    m_conc = qMax(1, concurrency);
    m_timeout = timeoutMs;
    m_total = int(m_last - m_next + 1);
    m_done = 0;
    m_active = 0;
    m_running = true;
    pump();
}

void PortScanner::stop()
{
    m_running = false;
    ++m_gen;
}

void PortScanner::pump()
{
    while (m_running && m_active < m_conc && m_next <= m_last) {
        const QString ip = QHostAddress(quint32(m_next++)).toString();
        const int gen = m_gen;
        ++m_active;

        auto *sock = new QTcpSocket(this);
        auto *timer = new QTimer(sock);      // удалится вместе с сокетом
        timer->setSingleShot(true);
        auto done = std::make_shared<bool>(false);

        auto finish = [this, sock, gen, ip, done](bool open) {
            if (*done)
                return;
            *done = true;
            sock->abort();
            sock->deleteLater();
            if (gen != m_gen)
                return;
            --m_active;
            ++m_done;
            if (open)
                emit found(ip);
            emit progress(m_done, m_total);
            pump();
        };

        connect(sock, &QTcpSocket::connected, this, [finish] { finish(true); });
        connect(sock, &QAbstractSocket::errorOccurred, this,
                [finish](QAbstractSocket::SocketError) { finish(false); });
        connect(timer, &QTimer::timeout, this, [finish] { finish(false); });

        timer->start(m_timeout);
        sock->connectToHost(ip, m_port);
    }

    if (m_running && m_next > m_last && m_active == 0) {
        m_running = false;
        emit finished();
    }
}

// ======================================================================
// core/recording_index.cpp
// ======================================================================
namespace {

SegmentInfo rowToSegment(const QSqlQuery &q)
{
    SegmentInfo s;
    s.id = q.value(0).toLongLong();
    s.cameraId = q.value(1).toString();
    s.path = q.value(2).toString();
    s.startMs = q.value(3).toLongLong();
    s.endMs = q.value(4).isNull() ? 0 : q.value(4).toLongLong();
    s.sizeBytes = q.value(5).toLongLong();
    return s;
}

const char *kSelectColumns = "SELECT id, camera_id, path, start_ms, end_ms, size_bytes FROM segments ";

} // namespace

RecordingIndex::~RecordingIndex() { close(); }

bool RecordingIndex::open(const QString &dbPath)
{
    close();
    m_conn = QStringLiteral("vms_idx_") + QUuid::createUuid().toString(QUuid::Id128);

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_conn);
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        m_lastError = db.lastError().text();
        return false;
    }

    QSqlQuery q(db);
    q.exec(QStringLiteral("PRAGMA journal_mode=WAL"));
    // FULL снижает риск потери последних изменений индекса при внезапном отключении питания.
    q.exec(QStringLiteral("PRAGMA synchronous=FULL"));
    q.exec(QStringLiteral("PRAGMA busy_timeout=10000"));

    const bool ok =
        q.exec(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS segments ("
            " id         INTEGER PRIMARY KEY AUTOINCREMENT,"
            " camera_id  TEXT    NOT NULL,"
            " path       TEXT    NOT NULL,"
            " start_ms   INTEGER NOT NULL,"
            " end_ms     INTEGER,"
            " size_bytes INTEGER NOT NULL DEFAULT 0)"))
        && q.exec(QStringLiteral(
            "CREATE INDEX IF NOT EXISTS idx_segments_cam_time "
            "ON segments(camera_id, start_ms)"));
    if (!ok)
        m_lastError = q.lastError().text();
    return ok;
}

void RecordingIndex::close()
{
    if (m_conn.isEmpty())
        return;
    {
        QSqlDatabase db = QSqlDatabase::database(m_conn, false);
        if (db.isOpen())
            db.close();
    }
    QSqlDatabase::removeDatabase(m_conn);
    m_conn.clear();
}

qint64 RecordingIndex::beginSegment(const QString &cameraId, const QString &path, qint64 startMs)
{
    QSqlQuery q(QSqlDatabase::database(m_conn));
    q.prepare(QStringLiteral("INSERT INTO segments(camera_id, path, start_ms) VALUES(?, ?, ?)"));
    q.addBindValue(cameraId);
    q.addBindValue(path);
    q.addBindValue(startMs);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return -1;
    }
    return q.lastInsertId().toLongLong();
}

bool RecordingIndex::endSegment(qint64 id, qint64 endMs, qint64 sizeBytes)
{
    if (id < 0)
        return false;
    QSqlQuery q(QSqlDatabase::database(m_conn));
    q.prepare(QStringLiteral("UPDATE segments SET end_ms = ?, size_bytes = ? WHERE id = ?"));
    q.addBindValue(endMs);
    q.addBindValue(sizeBytes);
    q.addBindValue(id);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

void RecordingIndex::closeDanglingSegments(const QString &cameraId)
{
    struct Row { qint64 id; QString path; qint64 startMs; };
    QList<Row> rows;
    {
        QSqlQuery q(QSqlDatabase::database(m_conn));
        q.prepare(QStringLiteral(
            "SELECT id, path, start_ms FROM segments WHERE camera_id = ? AND end_ms IS NULL"));
        q.addBindValue(cameraId);
        if (!q.exec()) {
            m_lastError = q.lastError().text();
            return;
        }
        while (q.next())
            rows.append({q.value(0).toLongLong(), q.value(1).toString(), q.value(2).toLongLong()});
    }

    for (const Row &r : rows) {
        const QFileInfo fi(r.path);
        QSqlQuery q(QSqlDatabase::database(m_conn));
        if (fi.exists() && fi.size() > 0) {
            const qint64 endMs = qMax(fi.lastModified().toMSecsSinceEpoch(), r.startMs);
            q.prepare(QStringLiteral("UPDATE segments SET end_ms = ?, size_bytes = ? WHERE id = ?"));
            q.addBindValue(endMs);
            q.addBindValue(fi.size());
            q.addBindValue(r.id);
        } else {
            q.prepare(QStringLiteral("DELETE FROM segments WHERE id = ?"));
            q.addBindValue(r.id);
        }
        q.exec();
    }
}

QList<SegmentInfo> RecordingIndex::segmentsInRange(const QString &cameraId, qint64 fromMs, qint64 toMs)
{
    QList<SegmentInfo> out;
    QSqlQuery q(QSqlDatabase::database(m_conn));
    q.prepare(QString::fromLatin1(kSelectColumns) +
              QStringLiteral("WHERE camera_id = ? AND start_ms < ? "
                             "AND (end_ms IS NULL OR end_ms > ?) ORDER BY start_ms"));
    q.addBindValue(cameraId);
    q.addBindValue(toMs);
    q.addBindValue(fromMs);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return out;
    }
    while (q.next())
        out.append(rowToSegment(q));
    return out;
}

SegmentInfo RecordingIndex::segmentAt(const QString &cameraId, qint64 ms)
{
    QSqlQuery q(QSqlDatabase::database(m_conn));
    q.prepare(QString::fromLatin1(kSelectColumns) +
              QStringLiteral("WHERE camera_id = ? AND start_ms <= ? "
                             "ORDER BY start_ms DESC LIMIT 1"));
    q.addBindValue(cameraId);
    q.addBindValue(ms);
    if (q.exec() && q.next()) {
        const SegmentInfo s = rowToSegment(q);
        if (s.endMs == 0 || ms <= s.endMs)
            return s;
    }
    return {};
}

// ======================================================================
// core/rtsp_probe.cpp
// ======================================================================
namespace {

struct RtspTemplate {
    const char *mainPath;
    const char *subPath;
};

// %CH% — номер канала, %ID1% / %ID2% — канал*100+1 / +2, %U% и %P% — логин и пароль
const RtspTemplate kRtspTemplates[] = {
    {"/Streaming/Channels/%ID1%", "/Streaming/Channels/%ID2%"},                       // Hikvision, HiWatch (новые)
    {"/h264/ch%CH%/main/av_stream", "/h264/ch%CH%/sub/av_stream"},                    // Hikvision, HiWatch (старые)
    {"/ISAPI/Streaming/channels/%ID1%", "/ISAPI/Streaming/channels/%ID2%"},
    {"/PSIA/Streaming/channels/%CH%", ""},                                            // очень старые
    {"/mpeg4/ch%CH%/main/av_stream", "/mpeg4/ch%CH%/sub/av_stream"},
    {"/cam/realmonitor?channel=%CH%&subtype=0", "/cam/realmonitor?channel=%CH%&subtype=1"},   // Dahua и OEM
    {"/user=%U%&password=%P%&channel=%CH%&stream=0.sdp?real_stream",
     "/user=%U%&password=%P%&channel=%CH%&stream=1.sdp?real_stream"},                 // Xiongmai (iCSee, часть Polyvision)
    {"/stream1", "/stream2"},
    {"/live/main", "/live/sub"},
    {"/11", "/12"},
    {"/1/h264major", "/1/h264minor"},
};
constexpr int kRtspTemplateCount = int(sizeof(kRtspTemplates) / sizeof(kRtspTemplates[0]));

QString expandRtspPath(const char *fmt, int channel, const QString &user, const QString &pass)
{
    QString s = QString::fromLatin1(fmt);
    s.replace(QLatin1String("%ID1%"), QString::number(channel * 100 + 1));
    s.replace(QLatin1String("%ID2%"), QString::number(channel * 100 + 2));
    s.replace(QLatin1String("%CH%"), QString::number(channel));
    s.replace(QLatin1String("%U%"), QString::fromUtf8(QUrl::toPercentEncoding(user)));
    s.replace(QLatin1String("%P%"), QString::fromUtf8(QUrl::toPercentEncoding(pass)));
    return s;
}

QByteArray md5Hex(const QByteArray &d)
{
    return QCryptographicHash::hash(d, QCryptographicHash::Md5).toHex();
}

QString headerParam(const QString &www, const QString &name)
{
    const QRegularExpression re(name + QStringLiteral("=\"([^\"]*)\""), QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch m = re.match(www);
    return m.hasMatch() ? m.captured(1) : QString();
}

} // namespace

RtspProbe::RtspProbe(QObject *parent) : QObject(parent) {}

int RtspProbe::templateCount()
{
    return kRtspTemplateCount;
}

QString RtspProbe::buildUrl(int tmpl, const QString &host, quint16 port, const QString &user,
                            const QString &pass, int channel, bool sub)
{
    if (tmpl < 0 || tmpl >= kRtspTemplateCount)
        return QString();
    const char *fmt = sub ? kRtspTemplates[tmpl].subPath : kRtspTemplates[tmpl].mainPath;
    if (!fmt || !*fmt)
        return QString();
    const bool credsInPath = QString::fromLatin1(fmt).contains(QLatin1String("%U%"));
    QString url = QStringLiteral("rtsp://");
    if (!credsInPath && !user.isEmpty())
        url += QString::fromUtf8(QUrl::toPercentEncoding(user)) + QLatin1Char(':')
               + QString::fromUtf8(QUrl::toPercentEncoding(pass)) + QLatin1Char('@');
    url += QStringLiteral("%1:%2").arg(host).arg(port) + expandRtspPath(fmt, channel, user, pass);
    return url;
}

void RtspProbe::start(const QString &host, quint16 port, const QString &user, const QString &pass)
{
    cancel();
    m_host = host;
    m_port = port ? port : 554;
    m_user = user;
    m_pass = pass;
    m_index = 0;
    tryNext();
}

void RtspProbe::cancel()
{
    ++m_gen;
}

void RtspProbe::tryNext()
{
    if (m_index >= kRtspTemplateCount) {
        emit finished(false, -1, QStringLiteral("none"));
        return;
    }
    request(m_index, QByteArray(), false);
}

void RtspProbe::request(int tmpl, const QByteArray &authHeader, bool authed)
{
    const int gen = m_gen;
    const QString path = expandRtspPath(kRtspTemplates[tmpl].mainPath, 1, m_user, m_pass);
    m_lastUri = QStringLiteral("rtsp://%1:%2").arg(m_host).arg(m_port) + path;

    QByteArray req = "DESCRIBE " + m_lastUri.toUtf8() + " RTSP/1.0\r\nCSeq: 2\r\n"
                     "User-Agent: PSVMS\r\nAccept: application/sdp\r\n";
    if (!authHeader.isEmpty())
        req += authHeader + "\r\n";
    req += "\r\n";

    auto *sock = new QTcpSocket(this);
    auto buf = std::make_shared<QByteArray>();
    auto done = std::make_shared<bool>(false);
    auto finish = [this, sock, tmpl, authed, gen, done](int code, const QString &www) {
        if (*done)
            return;
        *done = true;
        sock->abort();
        sock->deleteLater();
        if (gen != m_gen)
            return;
        onResponse(tmpl, authed, code, www);
    };

    connect(sock, &QTcpSocket::connected, this, [sock, req] { sock->write(req); });
    connect(sock, &QTcpSocket::readyRead, this, [sock, buf, finish] {
        buf->append(sock->readAll());
        const int end = buf->indexOf("\r\n\r\n");
        if (end < 0)
            return;
        const QList<QByteArray> lines = buf->left(end).split('\n');
        int code = -1;
        QString www;
        if (!lines.isEmpty()) {
            const QList<QByteArray> parts = lines.first().trimmed().split(' ');
            if (parts.size() >= 2)
                code = parts.at(1).toInt();
        }
        for (const QByteArray &ln : lines) {
            if (ln.trimmed().toLower().startsWith("www-authenticate:")) {
                if (www.isEmpty() || ln.contains("Digest"))
                    www = QString::fromLatin1(ln.mid(ln.indexOf(':') + 1).trimmed());
            }
        }
        finish(code, www);
    });
    connect(sock, &QAbstractSocket::errorOccurred, this,
            [finish](QAbstractSocket::SocketError) { finish(-1, QString()); });
    QTimer::singleShot(2500, sock, [finish] { finish(-1, QString()); });
    sock->connectToHost(m_host, m_port);
}

void RtspProbe::onResponse(int tmpl, bool authed, int code, const QString &www)
{
    if (code == 200) {
        emit finished(true, tmpl, QString());
        return;
    }
    if (code == 401) {
        if (authed || m_user.isEmpty()) {
            emit finished(false, -1, QStringLiteral("auth"));
            return;
        }
        const QByteArray header = buildAuthHeader(www);
        if (header.isEmpty()) {
            emit finished(false, -1, QStringLiteral("auth"));
            return;
        }
        request(tmpl, header, true);
        return;
    }
    if (code < 0) {                       // нет ответа: порт закрыт или камера недоступна
        emit finished(false, -1, QStringLiteral("none"));
        return;
    }
    ++m_index;                            // 404 и другие ответы: путь не подходит, пробуем следующий
    tryNext();
}

QByteArray RtspProbe::buildAuthHeader(const QString &www) const
{
    if (www.startsWith(QLatin1String("Basic"), Qt::CaseInsensitive)) {
        const QByteArray cred = (m_user + QLatin1Char(':') + m_pass).toUtf8().toBase64();
        return "Authorization: Basic " + cred;
    }
    if (!www.startsWith(QLatin1String("Digest"), Qt::CaseInsensitive))
        return QByteArray();

    const QString realm = headerParam(www, QStringLiteral("realm"));
    const QString nonce = headerParam(www, QStringLiteral("nonce"));
    const QString qop = headerParam(www, QStringLiteral("qop"));
    const QByteArray ha1 = md5Hex((m_user + QLatin1Char(':') + realm + QLatin1Char(':') + m_pass).toUtf8());
    const QByteArray ha2 = md5Hex("DESCRIBE:" + m_lastUri.toUtf8());

    QByteArray response, extra;
    if (qop.contains(QLatin1String("auth"))) {
        const QByteArray cnonce = QByteArray::number(QRandomGenerator::global()->generate(), 16);
        response = md5Hex(ha1 + ':' + nonce.toUtf8() + ":00000001:" + cnonce + ":auth:" + ha2);
        extra = ", qop=auth, nc=00000001, cnonce=\"" + cnonce + "\"";
    } else {
        response = md5Hex(ha1 + ':' + nonce.toUtf8() + ':' + ha2);
    }
    return "Authorization: Digest username=\"" + m_user.toUtf8() + "\", realm=\"" + realm.toUtf8()
           + "\", nonce=\"" + nonce.toUtf8() + "\", uri=\"" + m_lastUri.toUtf8() + "\", response=\""
           + response + "\"" + extra;
}

// ======================================================================
// core/secure_store.cpp
// ======================================================================
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

namespace {

constexpr int kKeyBytes = 32;

QString keyFilePath()
{
    return AppPaths::dataDir() + QStringLiteral("/.psvms_key");
}

void hideFile(const QString &path)
{
#ifdef Q_OS_WIN
    SetFileAttributesW(reinterpret_cast<const wchar_t *>(path.utf16()), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
#else
    Q_UNUSED(path)
#endif
}

// Ключ создаётся при первом обращении и живёт отдельным файлом рядом с настройками.
const QByteArray &encryptionKey()
{
    static const QByteArray key = [] {
        QDir().mkpath(AppPaths::dataDir());
        const QString path = keyFilePath();
        QFile f(path);
        if (f.exists() && f.open(QIODevice::ReadOnly)) {
            const QByteArray data = QByteArray::fromBase64(f.readAll());
            if (data.size() == kKeyBytes)
                return data;
        }
        QByteArray fresh(kKeyBytes, '\0');
        for (char &c : fresh)
            c = char(QRandomGenerator::system()->bounded(256));
        QFile out(path);
        if (out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            out.write(fresh.toBase64());
            out.close();
            hideFile(path);
        }
        return fresh;
    }();
    return key;
}

// Ключевой поток произвольной длины из SHA-256(ключ || номер блока)
QByteArray keystream(const QByteArray &key, int length)
{
    QByteArray out;
    out.reserve(length);
    quint32 counter = 0;
    while (out.size() < length) {
        QByteArray block = key;
        block.append(reinterpret_cast<const char *>(&counter), sizeof(counter));
        out += QCryptographicHash::hash(block, QCryptographicHash::Sha256);
        ++counter;
    }
    out.truncate(length);
    return out;
}

} // namespace

namespace {

QByteArray legacyDecryptBytes(const QByteArray &data)
{
    const QByteArray ks = keystream(encryptionKey(), data.size());
    QByteArray out(data.size(), '\0');
    for (int i = 0; i < data.size(); ++i)
        out[i] = data[i] ^ ks[i];
    return out;
}

#ifdef Q_OS_WIN
bool dpapiProtect(const QByteArray &input, QByteArray *output)
{
    if (!output || input.isEmpty()) return false;
    DATA_BLOB inBlob;
    inBlob.pbData = reinterpret_cast<BYTE *>(const_cast<char *>(input.constData()));
    inBlob.cbData = static_cast<DWORD>(input.size());
    DATA_BLOB outBlob{};
    static const wchar_t description[] = L"PSVMS camera credentials";
    if (!CryptProtectData(&inBlob, description, nullptr, nullptr, nullptr,
                          CRYPTPROTECT_UI_FORBIDDEN, &outBlob))
        return false;
    *output = QByteArray(reinterpret_cast<const char *>(outBlob.pbData), int(outBlob.cbData));
    SecureZeroMemory(outBlob.pbData, outBlob.cbData);
    LocalFree(outBlob.pbData);
    return true;
}

bool dpapiUnprotect(const QByteArray &input, QByteArray *output)
{
    if (!output || input.isEmpty()) return false;
    DATA_BLOB inBlob;
    inBlob.pbData = reinterpret_cast<BYTE *>(const_cast<char *>(input.constData()));
    inBlob.cbData = static_cast<DWORD>(input.size());
    DATA_BLOB outBlob{};
    if (!CryptUnprotectData(&inBlob, nullptr, nullptr, nullptr, nullptr,
                            CRYPTPROTECT_UI_FORBIDDEN, &outBlob))
        return false;
    *output = QByteArray(reinterpret_cast<const char *>(outBlob.pbData), int(outBlob.cbData));
    SecureZeroMemory(outBlob.pbData, outBlob.cbData);
    LocalFree(outBlob.pbData);
    return true;
}
#endif

} // namespace

QString SecureStore::encrypt(const QString &plain)
{
    if (plain.isEmpty()) return QString();
    const QByteArray data = plain.toUtf8();
#ifdef Q_OS_WIN
    QByteArray protectedData;
    if (!dpapiProtect(data, &protectedData)) {
        writePsvmsLog(QStringLiteral("SecureStore: DPAPI encryption failed; using legacy compatibility encryption."));
        const QByteArray ks = keystream(encryptionKey(), data.size());
        QByteArray out(data.size(), '\0');
        for (int i = 0; i < data.size(); ++i) out[i] = data[i] ^ ks[i];
        return QStringLiteral("legacy:v1:") + QString::fromLatin1(out.toBase64());
    }
    return QString::fromLatin1(protectedData.toBase64());
#else
    // На других платформах сохраняем совместимость, но не выдаём это за DPAPI.
    const QByteArray ks = keystream(encryptionKey(), data.size());
    QByteArray out(data.size(), '\0');
    for (int i = 0; i < data.size(); ++i) out[i] = data[i] ^ ks[i];
    return QStringLiteral("legacy:v1:") + QString::fromLatin1(out.toBase64());
#endif
}

QString SecureStore::decrypt(const QString &cipherB64)
{
    if (cipherB64.isEmpty()) return QString();
    const QByteArray data = QByteArray::fromBase64(cipherB64.toLatin1());
    if (data.isEmpty()) return QString();
#ifdef Q_OS_WIN
    QByteArray plain;
    if (!dpapiUnprotect(data, &plain)) {
        writePsvmsLog(QStringLiteral("SecureStore: DPAPI decryption failed; refusing to treat the value as plaintext."));
        return QString();
    }
    const QString result = QString::fromUtf8(plain);
    SecureZeroMemory(plain.data(), size_t(plain.size()));
    return result;
#else
    const QByteArray plain = legacyDecryptBytes(data);
    return QString::fromUtf8(plain);
#endif
}

QString SecureStore::decryptLegacy(const QString &cipherB64)
{
    if (cipherB64.isEmpty()) return QString();
    const QByteArray data = QByteArray::fromBase64(cipherB64.toLatin1());
    if (data.isEmpty()) return QString();
    const QByteArray plain = legacyDecryptBytes(data);
    return QString::fromUtf8(plain);
}

// ======================================================================
// core/stream_presets.cpp
// ======================================================================
namespace {

QString kindName(int index)   // 0 — основной, 1 — подпоток, 2 — третий
{
    switch (index) {
    case 0:  return T("Основной поток");
    case 1:  return T("Подпоток");
    default: return T("Третий поток");
    }
}

} // namespace

QList<StreamOption> streamOptionsFor(const QString &url)
{
    QList<StreamOption> out;
    if (url.isEmpty())
        return out;

    // Hikvision / HiWatch: номер = канал * 100 + поток (1 — основной, 2 — подпоток, 3 — третий)
    static const QRegularExpression hik(QStringLiteral("^(.*/Streaming/Channels/)(\\d+)(.*)$"),
                                        QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch m = hik.match(url);
    if (m.hasMatch()) {
        const int channel = m.captured(2).toInt() / 100;
        if (channel >= 1) {
            for (int s = 1; s <= 3; ++s) {
                const int id = channel * 100 + s;
                StreamOption o;
                o.label = kindName(s - 1) + QStringLiteral(" (%1)").arg(id);
                o.url = m.captured(1) + QString::number(id) + m.captured(3);
                out.append(o);
            }
            return out;
        }
    }

    // Старые Hikvision / HiWatch: .../h264/ch1/main/av_stream и .../sub/av_stream
    static const QRegularExpression oldHik(QStringLiteral("^(.*/(?:h264|mpeg4)/ch\\d+/)(main|sub)(/av_stream.*)$"));
    m = oldHik.match(url);
    if (m.hasMatch()) {
        const char *kinds[2] = {"main", "sub"};
        for (int s = 0; s <= 1; ++s) {
            StreamOption o;
            o.label = kindName(s) + QStringLiteral(" (%1)").arg(QLatin1String(kinds[s]));
            o.url = m.captured(1) + QLatin1String(kinds[s]) + m.captured(3);
            out.append(o);
        }
        return out;
    }

    // Dahua и совместимые: ...subtype=0 / 1 / 2
    static const QRegularExpression dahua(QStringLiteral("^(.*[?&]subtype=)(\\d)(.*)$"));
    m = dahua.match(url);
    if (m.hasMatch()) {
        for (int s = 0; s <= 2; ++s) {
            StreamOption o;
            o.label = kindName(s) + QStringLiteral(" (subtype=%1)").arg(s);
            o.url = m.captured(1) + QString::number(s) + m.captured(3);
            out.append(o);
        }
        return out;
    }

    // iCSee / Xiongmai: ...stream=0 (основной) или stream=1 (подпоток)
    static const QRegularExpression icsee(QStringLiteral("^(.*[?&/]stream=)(\\d)(.*)$"));
    m = icsee.match(url);
    if (m.hasMatch()) {
        for (int s = 0; s <= 1; ++s) {
            StreamOption o;
            o.label = kindName(s) + QStringLiteral(" (stream=%1)").arg(s);
            o.url = m.captured(1) + QString::number(s) + m.captured(3);
            out.append(o);
        }
    }
    return out;
}

// ======================================================================
// core/sys_stats.cpp
// ======================================================================
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif

namespace {
// Разница между двумя замерами FILETIME в «тиках» по 100 нс
qint64 ftDiff(const FILETIME &a, const FILETIME &b)
{
    auto toI64 = [](const FILETIME &f) { return (qint64(f.dwHighDateTime) << 32) | f.dwLowDateTime; };
    return toI64(b) - toI64(a);
}
} // namespace

SysStats readSysStats()
{
    SysStats s;

    static FILETIME prevIdle{}, prevKernel{}, prevUser{};
    static bool havePrev = false;
    FILETIME idle, kernel, user;
    if (GetSystemTimes(&idle, &kernel, &user)) {
        if (havePrev) {
            const qint64 idleDiff = ftDiff(prevIdle, idle);
            const qint64 kernelDiff = ftDiff(prevKernel, kernel);
            const qint64 userDiff = ftDiff(prevUser, user);
            const qint64 total = kernelDiff + userDiff;   // kernel-время уже включает idle
            if (total > 0)
                s.cpuPercent = 100.0 * double(total - idleDiff) / double(total);
        }
        prevIdle = idle;
        prevKernel = kernel;
        prevUser = user;
        havePrev = true;
    }

    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    if (GlobalMemoryStatusEx(&mem)) {
        s.memPercent = double(mem.dwMemoryLoad);
        s.memTotalMb = qint64(mem.ullTotalPhys / (1024 * 1024));
        s.memUsedMb = s.memTotalMb - qint64(mem.ullAvailPhys / (1024 * 1024));
    }
    return s;
}
#else
SysStats readSysStats()
{
    return SysStats();   // сборка вне Windows: показатели недоступны
}
#endif

// ======================================================================
// core/users.cpp
// ======================================================================
namespace {

constexpr int kUserIterations = 600000;
constexpr int kUserKeyLen = 32;
QString g_currentUserId;

struct UserRec {
    QString id, name, role, salt, hash, remember;
    bool enabled = true;
    int iterations = kUserIterations;
};

QList<UserRec> loadUserRecs()
{
    QList<UserRec> out;
    QSettings s;
    const int n = s.beginReadArray(QStringLiteral("users"));
    for (int i = 0; i < n; ++i) {
        s.setArrayIndex(i);
        UserRec r;
        r.id = s.value(QStringLiteral("id")).toString();
        r.name = s.value(QStringLiteral("name")).toString();
        r.role = s.value(QStringLiteral("role")).toString();
        r.salt = s.value(QStringLiteral("salt")).toString();
        r.hash = s.value(QStringLiteral("hash")).toString();
        r.remember = s.value(QStringLiteral("remember")).toString();
        r.enabled = s.value(QStringLiteral("enabled"), true).toBool();
        r.iterations = s.value(QStringLiteral("iterations"), kUserIterations).toInt();
        if (!r.id.isEmpty())
            out.append(r);
    }
    s.endArray();
    return out;
}

void saveUserRecs(const QList<UserRec> &v)
{
    QSettings s;
    s.remove(QStringLiteral("users"));
    s.beginWriteArray(QStringLiteral("users"), int(v.size()));
    for (int i = 0; i < v.size(); ++i) {
        s.setArrayIndex(i);
        const UserRec &r = v.at(i);
        s.setValue(QStringLiteral("id"), r.id);
        s.setValue(QStringLiteral("name"), r.name);
        s.setValue(QStringLiteral("role"), r.role);
        s.setValue(QStringLiteral("salt"), r.salt);
        s.setValue(QStringLiteral("hash"), r.hash);
        s.setValue(QStringLiteral("remember"), r.remember);
        s.setValue(QStringLiteral("enabled"), r.enabled);
        s.setValue(QStringLiteral("iterations"), r.iterations);
    }
    s.endArray();
    s.sync();
}

QByteArray randomUserBytes(int n)
{
    QByteArray b(n, '\0');
    for (char &c : b)
        c = char(QRandomGenerator::system()->bounded(256));
    return b;
}

QByteArray deriveUserKey(const QString &password, const QByteArray &salt, int iterations)
{
    return QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256, password.toUtf8(), salt,
                                              iterations, kUserKeyLen);
}

bool userBytesEqual(const QByteArray &a, const QByteArray &b)
{
    if (a.size() != b.size())
        return false;
    unsigned char diff = 0;
    for (int i = 0; i < a.size(); ++i)
        diff |= static_cast<unsigned char>(a.at(i)) ^ static_cast<unsigned char>(b.at(i));
    return diff == 0;
}

QString makeUserId()
{
    return QStringLiteral("u") + QUuid::createUuid().toString(QUuid::Id128).left(8);
}

UserInfo toUserInfo(const UserRec &r)
{
    UserInfo u;
    u.id = r.id;
    u.name = r.name;
    u.role = r.role;
    u.hasPassword = !r.hash.isEmpty();
    u.enabled = r.enabled;
    return u;
}

void applyUserPassword(UserRec &r, const QString &password)
{
    r.remember.clear();
    if (password.isEmpty()) {
        r.salt.clear();
        r.hash.clear();
        return;
    }
    const QByteArray salt = randomUserBytes(16);
    r.iterations = kUserIterations;
    r.salt = QString::fromLatin1(salt.toBase64());
    r.hash = QString::fromLatin1(deriveUserKey(password, salt, r.iterations).toBase64());
}

QString tokenHash(const QString &token)
{
    return QString::fromLatin1(QCryptographicHash::hash(token.toLatin1(), QCryptographicHash::Sha256).toBase64());
}

} // namespace

QList<UserInfo> Users::all()
{
    QList<UserInfo> out;
    for (const UserRec &r : loadUserRecs())
        out.append(toUserInfo(r));
    return out;
}

UserInfo Users::find(const QString &id)
{
    for (const UserRec &r : loadUserRecs())
        if (r.id == id)
            return toUserInfo(r);
    return UserInfo();
}

void Users::ensureDefault()
{
    QList<UserRec> users = loadUserRecs();
    auto hasRole = [&users](const QString &role) {
        for (const UserRec &u : users)
            if (u.role == role) return true;
        return false;
    };
    if (!hasRole(QStringLiteral("guest"))) {
        UserRec guest;
        guest.id = makeUserId();
        guest.name = T("Гость");
        guest.role = QStringLiteral("guest");
        users.prepend(guest);
    }
    if (!hasRole(QStringLiteral("admin"))) {
        UserRec admin;
        admin.id = makeUserId();
        admin.name = QStringLiteral("Administrator");
        admin.role = QStringLiteral("admin");
        users.append(admin);
    } else {
        for (UserRec &u : users) {
            if (u.role == QStringLiteral("admin") &&
                (u.name.compare(QStringLiteral("Администратор"), Qt::CaseInsensitive) == 0 ||
                 u.name.compare(QStringLiteral("Administrator"), Qt::CaseInsensitive) == 0)) {
                u.name = QStringLiteral("Administrator");
                break;
            }
        }
    }
    if (!hasRole(QStringLiteral("developer"))) {
        UserRec developer;
        developer.id = makeUserId();
        developer.name = T("Разработчик");
        developer.role = QStringLiteral("developer");
        users.append(developer); // пароль задаётся при первом входе
    }
    for (UserRec &u : users) {
        if (u.role == QLatin1String("guest") || u.role == QLatin1String("developer"))
            u.enabled = true;
    }
    saveUserRecs(users);
}

bool Users::needsLoginDialog()
{
    return true; // каждый запуск начинается с выбора гостя или учётной записи
}

QString Users::add(const QString &name, const QString &role, const QString &password, QString *error)
{
    if (!Users::current().isAdmin()) { if (error) *error = T("Недостаточно прав: требуется администратор."); SecurityAudit::write(QStringLiteral("permission_denied"), QStringLiteral("users.add")); return QString(); }
    const QString n = name.trimmed();
    if (n.isEmpty()) {
        if (error) *error = T("Укажите имя пользователя.");
        return QString();
    }
    QList<UserRec> v = loadUserRecs();
    for (const UserRec &r : v) {
        if (r.name.compare(n, Qt::CaseInsensitive) == 0) {
            if (error) *error = T("Пользователь с таким именем уже есть.");
            return QString();
        }
    }
    if (!password.isEmpty() && password.size() < kMinPassword) {
        if (error) *error = T("Пароль слишком короткий (минимум %1).").arg(kMinPassword);
        return QString();
    }
    UserRec r;
    r.id = makeUserId();
    r.name = n;
    r.role = role == QLatin1String("admin") ? QStringLiteral("admin") : QStringLiteral("operator");
    r.enabled = true;
    applyUserPassword(r, password);
    v.append(r);
    saveUserRecs(v);
    SecurityAudit::write(QStringLiteral("user_created"), r.id + QStringLiteral(" role=") + r.role);
    return r.id;
}

bool Users::remove(const QString &id, QString *error)
{
    if (!Users::current().isAdmin()) { if (error) *error = T("Недостаточно прав: требуется администратор."); SecurityAudit::write(QStringLiteral("permission_denied"), QStringLiteral("users.remove")); return false; }
    QList<UserRec> v = loadUserRecs();
    if (v.size() <= 1) {
        if (error) *error = T("Нельзя удалить единственного пользователя.");
        return false;
    }
    if (id == g_currentUserId) {
        if (error) *error = T("Нельзя удалить пользователя, под которым выполнен вход.");
        return false;
    }
    int idx = -1, admins = 0;
    for (int i = 0; i < v.size(); ++i) {
        if (v.at(i).role == QLatin1String("admin"))
            ++admins;
        if (v.at(i).id == id)
            idx = i;
    }
    if (idx < 0)
        return false;
    if (v.at(idx).role == QLatin1String("developer") || v.at(idx).role == QLatin1String("guest")) {
        if (error) *error = v.at(idx).role == QLatin1String("guest")
            ? T("Системную учётную запись «Гость» нельзя удалить.")
            : T("Учётную запись «Разработчик» нельзя удалить.");
        return false;
    }
    if (v.at(idx).role == QLatin1String("admin") && admins <= 1) {
        if (error) *error = T("Нельзя удалить последнего администратора.");
        return false;
    }
    v.removeAt(idx);
    saveUserRecs(v);
    SecurityAudit::write(QStringLiteral("user_removed"), id);
    return true;
}

bool Users::setPassword(const QString &id, const QString &password, QString *error)
{
    if (!Users::current().isAdmin() && id != Users::current().id) { if (error) *error = T("Недостаточно прав."); SecurityAudit::write(QStringLiteral("permission_denied"), QStringLiteral("users.setPassword")); return false; }
    if (Users::find(id).isDeveloper() && !Users::current().isDeveloper()) {
        if (error) *error = T("Изменять пароль учётной записи «Разработчик» может только сам разработчик.");
        return false;
    }
    QList<UserRec> v = loadUserRecs();
    for (UserRec &r : v) {
        if (r.id == id) {
            if (r.role == QLatin1String("developer") && password.isEmpty()) {
                if (error) *error = T("Для учётной записи «Разработчик» пароль обязателен.");
                return false;
            }
            if (!password.isEmpty() && password.size() < kMinPassword) {
                if (error) *error = T("Пароль слишком короткий (минимум %1).").arg(kMinPassword);
                return false;
            }
            applyUserPassword(r, password);
            saveUserRecs(v);
            SecurityAudit::write(QStringLiteral("user_password_changed"), id);
            return true;
        }
    }
    return false;
}

bool Users::setRole(const QString &id, const QString &role, QString *error)
{
    if (!Users::current().isAdmin()) { if (error) *error = T("Недостаточно прав: требуется администратор."); SecurityAudit::write(QStringLiteral("permission_denied"), QStringLiteral("users.setRole")); return false; }
    const QString newRole = role == QLatin1String("admin") ? QStringLiteral("admin") : QStringLiteral("operator");
    const UserInfo targetUser = Users::find(id);
    if (targetUser.isDeveloper() || targetUser.isGuest()) {
        if (error) *error = targetUser.isGuest()
            ? T("Роль системной учётной записи «Гость» нельзя изменить.")
            : T("Роль учётной записи «Разработчик» нельзя изменить.");
        return false;
    }
    QList<UserRec> v = loadUserRecs();
    int idx = -1, admins = 0;
    for (int i = 0; i < v.size(); ++i) {
        if (v.at(i).role == QLatin1String("admin"))
            ++admins;
        if (v.at(i).id == id)
            idx = i;
    }
    if (idx < 0)
        return false;
    if (v.at(idx).role == QLatin1String("admin") && newRole != QLatin1String("admin") && admins <= 1) {
        if (error) *error = T("Нельзя понизить последнего администратора.");
        return false;
    }
    v[idx].role = newRole;
    saveUserRecs(v);
    SecurityAudit::write(QStringLiteral("user_role_changed"), id + QStringLiteral(" role=") + newRole);
    return true;
}

bool Users::setEnabled(const QString &id, bool enabled, QString *error)
{
    if (!Users::current().isAdmin()) {
        if (error) *error = T("Недостаточно прав: требуется администратор.");
        SecurityAudit::write(QStringLiteral("permission_denied"), QStringLiteral("users.setEnabled"));
        return false;
    }
    if (id == Users::current().id) {
        if (error) *error = T("Нельзя отключить учётную запись, под которой выполнен вход.");
        return false;
    }
    QList<UserRec> v = loadUserRecs();
    int idx = -1;
    int enabledAdmins = 0;
    for (int i = 0; i < v.size(); ++i) {
        if (v.at(i).role == QLatin1String("admin") && v.at(i).enabled)
            ++enabledAdmins;
        if (v.at(i).id == id)
            idx = i;
    }
    if (idx < 0) {
        if (error) *error = T("Пользователь не найден.");
        return false;
    }
    if (v.at(idx).role == QLatin1String("guest") || v.at(idx).role == QLatin1String("developer")) {
        if (error) *error = T("Системные учётные записи «Гость» и «Разработчик» нельзя отключить.");
        return false;
    }
    if (!enabled && v.at(idx).role == QLatin1String("admin") && enabledAdmins <= 1) {
        if (error) *error = T("Нельзя отключить последнего активного администратора.");
        return false;
    }
    v[idx].enabled = enabled;
    v[idx].remember.clear();
    saveUserRecs(v);
    SecurityAudit::write(enabled ? QStringLiteral("user_enabled") : QStringLiteral("user_disabled"), id);
    return true;
}

bool Users::verify(const QString &id, const QString &password)
{
    for (const UserRec &r : loadUserRecs()) {
        if (r.id != id)
            continue;
        if (!r.enabled)
            return false;
        if (r.hash.isEmpty())
            return true;                              // пароль не задан
        const QByteArray salt = QByteArray::fromBase64(r.salt.toLatin1());
        const QByteArray stored = QByteArray::fromBase64(r.hash.toLatin1());
        return userBytesEqual(deriveUserKey(password, salt, r.iterations), stored);
    }
    return false;
}

QString Users::issueRememberToken(const QString &id)
{
    QList<UserRec> v = loadUserRecs();
    for (UserRec &r : v) {
        if (r.id == id) {
            const QString token = QString::fromLatin1(randomUserBytes(32).toBase64());
            r.remember = tokenHash(token);
            saveUserRecs(v);
            QSettings s;
            s.setValue(QStringLiteral("session/userId"), id);
            s.setValue(QStringLiteral("session/token"), token);
            return token;
        }
    }
    return QString();
}

bool Users::consumeRememberedLogin(QString *userId)
{
    QSettings s;
    const QString id = s.value(QStringLiteral("session/userId")).toString();
    const QString token = s.value(QStringLiteral("session/token")).toString();
    if (id.isEmpty() || token.isEmpty())
        return false;
    for (const UserRec &r : loadUserRecs()) {
        if (r.id == id && !r.remember.isEmpty()
            && userBytesEqual(tokenHash(token).toLatin1(), r.remember.toLatin1())) {
            if (userId)
                *userId = id;
            return true;
        }
    }
    return false;
}

void Users::forgetRemembered()
{
    QSettings s;
    const QString id = s.value(QStringLiteral("session/userId")).toString();
    s.remove(QStringLiteral("session/userId"));
    s.remove(QStringLiteral("session/token"));
    if (id.isEmpty())
        return;
    QList<UserRec> v = loadUserRecs();
    for (UserRec &r : v)
        if (r.id == id)
            r.remember.clear();
    saveUserRecs(v);
}

void Users::setCurrent(const QString &id)
{
    g_currentUserId = id;
}

UserInfo Users::current()
{
    return find(g_currentUserId);
}

QString Users::lastUserId()
{
    return QSettings().value(QStringLiteral("session/lastUser")).toString();
}

void Users::setLastUserId(const QString &id)
{
    QSettings().setValue(QStringLiteral("session/lastUser"), id);
}

// ======================================================================
// app/win_integration.h
// ======================================================================
#include <QWidget>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dwmapi.h>
#endif

// Windows 10/11: тёмная рамка заголовка окна под цвет программы
inline void applyDarkTitleBar(QWidget *w)
{
#ifdef Q_OS_WIN
    if (!w || !w->isWindow())
        return;
    const HWND hwnd = reinterpret_cast<HWND>(w->winId());
    const BOOL on = TRUE;
    DwmSetWindowAttribute(hwnd, 20, &on, sizeof(on));   // DWMWA_USE_IMMERSIVE_DARK_MODE (Windows 10 2004+ и 11)
    DwmSetWindowAttribute(hwnd, 19, &on, sizeof(on));   // то же для ранних сборок Windows 10
#else
    Q_UNUSED(w)
#endif
}

// ======================================================================
// app/main.cpp
// ======================================================================
// Windows 10/11: у каждого окна программы тёмная рамка заголовка
class DarkTitleFilter : public QObject {
public:
    using QObject::QObject;
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (e->type() == QEvent::Show && o->isWidgetType()) {
            auto *w = static_cast<QWidget *>(o);
            if (w->windowType() == Qt::Window || w->windowType() == Qt::Dialog)
                applyDarkTitleBar(w);
        }
        return false;
    }
};

namespace {
QMutex g_psvmsLogMutex;
QString g_psvmsLogFilePath;

QString psvmsLogTypeName(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:    return QStringLiteral("DEBUG");
    case QtInfoMsg:     return QStringLiteral("INFO");
    case QtWarningMsg:  return QStringLiteral("WARNING");
    case QtCriticalMsg: return QStringLiteral("CRITICAL");
    case QtFatalMsg:    return QStringLiteral("FATAL");
    }
    return QStringLiteral("UNKNOWN");
}
}

// Запись журнала в папку logs рядом с исполняемым файлом.
// Открываем файл на каждую запись: сообщения не теряются при аварийном выходе.
static void writePsvmsLog(const QString &message)
{
    QMutexLocker locker(&g_psvmsLogMutex);
    if (g_psvmsLogFilePath.isEmpty())
        return;
    QFile file(g_psvmsLogFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return;
    const QByteArray line = QStringLiteral("[%1] %2\r\n")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz")), message)
        .toUtf8();
    file.write(line);
    file.flush();
}

static void psvmsQtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    QString line = QStringLiteral("%1: %2").arg(psvmsLogTypeName(type), message);
    if (context.file && *context.file)
        line += QStringLiteral(" (файл: %1:%2)").arg(QString::fromUtf8(context.file)).arg(context.line);
    writePsvmsLog(line);
    if (type == QtFatalMsg)
        std::abort();
}

static void psvmsTerminateHandler()
{
    QString details = QStringLiteral("НЕОБРАБОТАННОЕ ИСКЛЮЧЕНИЕ C++ / std::terminate");
    const std::exception_ptr ex = std::current_exception();
    if (ex) {
        try {
            std::rethrow_exception(ex);
        } catch (const std::exception &e) {
            details += QStringLiteral(": %1").arg(QString::fromLocal8Bit(e.what()));
        } catch (...) {
            details += QStringLiteral(": неизвестное исключение");
        }
    }
    writePsvmsLog(details);
    std::abort();
}

#ifdef Q_OS_WIN
static LONG WINAPI psvmsUnhandledExceptionFilter(EXCEPTION_POINTERS *info)
{
    char buffer[256] = {};
    const DWORD code = info && info->ExceptionRecord ? info->ExceptionRecord->ExceptionCode : 0;
    const void *address = info && info->ExceptionRecord ? info->ExceptionRecord->ExceptionAddress : nullptr;
    const int len = std::snprintf(buffer, sizeof(buffer),
        "[CRASH] Необработанное исключение Windows: code=0x%08lX address=%p\r\n",
        static_cast<unsigned long>(code), address);
    if (!g_psvmsLogFilePath.isEmpty() && len > 0) {
        HANDLE file = CreateFileW(reinterpret_cast<LPCWSTR>(g_psvmsLogFilePath.utf16()),
            FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file != INVALID_HANDLE_VALUE) {
            DWORD written = 0;
            WriteFile(file, buffer, static_cast<DWORD>(len), &written, nullptr);
            FlushFileBuffers(file);
            CloseHandle(file);
        }
    }
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

static void initializePsvmsLogging()
{
    QString logDir = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("logs"));
    if (!QDir().mkpath(logDir)) {
        // Для установки в защищённую папку Windows используем резервный
        // пользовательский каталог, если рядом с exe нет прав на запись.
        logDir = QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation))
            .filePath(QStringLiteral("logs"));
        if (!QDir().mkpath(logDir)) {
            qWarning() << "Не удалось создать каталог журнала:" << logDir;
            return;
        }
    }
    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss"));
    g_psvmsLogFilePath = QDir(logDir).filePath(QStringLiteral("PSVMS_%1.log").arg(stamp));
    qInstallMessageHandler(psvmsQtMessageHandler);
    std::set_terminate(psvmsTerminateHandler);
#ifdef Q_OS_WIN
    SetUnhandledExceptionFilter(psvmsUnhandledExceptionFilter);
#endif
    writePsvmsLog(QStringLiteral("============================================================"));
    writePsvmsLog(QStringLiteral("Запуск PSVMS; журнал: %1").arg(g_psvmsLogFilePath));
}

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(QStringLiteral("PSVMS"));
    QCoreApplication::setApplicationName(QStringLiteral("PSVMS"));

    // Масштаб интерфейса задаётся в настройках и применяется при запуске (мониторы с высоким разрешением)
    {
        QSettings settings;
        const double presetScale = settings.value(QStringLiteral("ui/scale"), 0.0).toDouble();
        const double manualScale = qBound(75, settings.value(QStringLiteral("ui/manualInterfacePercent"), 100).toInt(), 200) / 100.0;
        const double scale = (presetScale > 0.0 ? presetScale : 1.0) * manualScale;
        if (scale >= 0.75 && scale <= 4.0 && qAbs(scale - 1.0) > 0.001)
            qputenv("QT_SCALE_FACTOR", QByteArray::number(scale, 'f', 3));
    }
    // Дробное масштабирование Windows 10/11 (125%, 150%) без искажений
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);
    initializePsvmsLogging();
    app.setApplicationDisplayName(App::title());
    I18n::init();      // язык из настроек или по языку системы
    Cfg::reload();

    app.setStyle(QStringLiteral("Fusion"));
    QString uiFamily = QStringLiteral("Segoe UI Variable");
    const QStringList installedFonts = QFontDatabase::families();
    if (!installedFonts.contains(uiFamily, Qt::CaseInsensitive)) {
        uiFamily = installedFonts.contains(QStringLiteral("Inter"), Qt::CaseInsensitive)
            ? QStringLiteral("Inter") : QStringLiteral("Segoe UI");
    }
    QFont uiFont(uiFamily, 10);
    const int manualTextPercent = qBound(75, QSettings().value(QStringLiteral("ui/manualTextPercent"), 100).toInt(), 200);
    uiFont.setPointSizeF(uiFont.pointSizeF() * manualTextPercent / 100.0);
    uiFont.setStyleStrategy(QFont::PreferAntialias);
    uiFont.setHintingPreference(QFont::PreferFullHinting);
    app.setFont(uiFont);
    QString effectiveStyle = appStyleSheet();
    const int elementPercent = qBound(75, QSettings().value(QStringLiteral("ui/manualElementsPercent"), 100).toInt(), 200);
    const double elementFactor = elementPercent / 100.0;
    // Компактные элементы управления: сохраняем масштабирование, но убираем лишнюю высоту и ширину.
    const int controlMinHeight = qMax(21, qRound(26 * elementFactor));
    const int controlPadV = qMax(1, qRound(3 * elementFactor));
    const int controlPadH = qMax(4, qRound(9 * elementFactor));
    effectiveStyle += QStringLiteral("\nQPushButton, QToolButton, QComboBox, QLineEdit, QSpinBox, QDoubleSpinBox { min-height:%1px; padding:%2px %3px; }\n")
        .arg(controlMinHeight).arg(controlPadV).arg(controlPadH);
    app.setStyleSheet(effectiveStyle);
    DarkTitleFilter titleFilter;
    app.installEventFilter(&titleFilter);

    // После заставки показывается выбор учётной записи; гостевой режим предлагается первым.
    // Пользователи создаются без пароля, кроме защищённой учётной записи Разработчик.

    // ---- заставка ----
    QElapsedTimer clock;
    clock.start();
    QSplashScreen *splash = createSplash();
    Users::ensureDefault();
    while (clock.elapsed() < 3500) {               // заставка видна ровно 3,5 секунды
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(15);
    }
    finishSplash(splash);

    int rc = 0;
    std::unique_ptr<CameraManager> manager;
    for (;;) {
        // При каждом запуске пользователь выбирает гостевой режим или учётную запись.
        if (!authenticate(nullptr))
            break;
        if (!manager)
            manager = std::make_unique<CameraManager>();   // при выключенном автозапуске камеры не стартуют

        {
            MainWindow window(manager.get());
            // При каждом запуске открываем главное окно развёрнутым, но не
            // в эксклюзивном fullscreen: системная строка заголовка и крестик
            // закрытия остаются доступными.
            window.showMaximized();
            rc = app.exec();
        }
        if (I18n::takeRestartRequest())      // смена языка: окно создаётся заново
            continue;
        if (App::takeLogoutRequest()) {      // выход из учётной записи: снова спрашиваем вход
            Users::forgetRemembered();
            Users::setCurrent(QString());
            continue;
        }
        break;
    }
    writePsvmsLog(QStringLiteral("Завершение PSVMS, код возврата: %1").arg(rc));
    return rc;
}

// Код moc для классов с Q_OBJECT (генерируется AUTOMOC)
#include "psvms.moc"
