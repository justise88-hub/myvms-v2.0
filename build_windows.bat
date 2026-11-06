@echo off
setlocal
REM Укажите пути к Qt MSVC 2022 и vcpkg.
set QT_DIR=C:\Qt\6.8.1\msvc2022_64
set VCPKG_ROOT=C:\vcpkg
if not exist "%QT_DIR%\bin\windeployqt.exe" (echo Qt not found in %QT_DIR% & exit /b 1)
if not exist "%VCPKG_ROOT%\vcpkg.exe" (echo vcpkg not found in %VCPKG_ROOT% & exit /b 1)
"%VCPKG_ROOT%\vcpkg.exe" install "ffmpeg[core,avcodec,avformat]:x64-windows" openssl:x64-windows || exit /b 1
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="%QT_DIR%" -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" || exit /b 1
cmake --build build --config Release || exit /b 1
"%QT_DIR%\bin\windeployqt.exe" --release --no-translations build\Release\PSVMS.exe || exit /b 1
echo PSVMS ready in build\Release
