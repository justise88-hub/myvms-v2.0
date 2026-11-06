# PSVMS — Windows VMS project

This package contains the supplied C++ source and build configuration.

## Build with GitHub Actions
1. Create a GitHub repository.
2. Upload all files in this folder, including the hidden `.github` directory.
3. Open **Actions** → **Build PSVMS for Windows** → **Run workflow**.
4. When it succeeds, download the `PSVMS-windows` artifact.

## Local build
Install Visual Studio 2022 with Desktop development with C++, Qt 6.8.1 for MSVC 2022 x64, CMake, and vcpkg.
Edit `QT_DIR` and `VCPKG_ROOT` in `build_windows.bat`, then run it.

## Important
A Windows executable is not included. The C++ source has not yet been compiled or runtime-tested in this environment. A successful build must be confirmed by the Windows build workflow or a local Windows build.


## Значок Windows
В проект добавлены `PSVMS.ico` (многоразмерный значок Windows), `app_icon.rc` и подключение ресурса в `CMakeLists.txt`. При пересборке Windows EXE получит значок PSVMS. Для ярлыка Windows: Свойства ярлыка → Сменить значок → выбрать `PSVMS.ico`.

## Обновлённый интерфейс PSVMS

В этой версии обновлена тема приложения в графитово-серебристом стиле с жёлтыми акцентами, добавлен фирменный знак в шапку боковой панели, а панель PTZ перенесена в отдельную правую колонку рядом с видеомозаикой. Значок Windows подключён через `app_icon.rc`; PNG-логотип встраивается через `branding.qrc`.

После замены файлов в репозитории пересоберите проект через GitHub Actions. Изменения интерфейса подготовлены в исходниках, но сборка и проверка на Windows ещё не выполнялись.


## Premium UI refresh
- Graphite/silver/gold controls and Segoe UI typography.
- Embedded city-at-night wallpaper in the navigation rail.
- Larger PSVMS emblem.
- Full-screen button moved to the bottom of the left camera rail; F11 remains available.
