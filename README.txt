PSVMS — modified source project

The modified application source is psvms.cpp.
Requirements: Qt 6.5+ (Widgets, Multimedia, MultimediaWidgets, SQL, Network), FFmpeg development libraries, CMake 3.21+, and a C++17 compiler.

Windows / Visual Studio 2022:
1. Edit QT_DIR and VCPKG_ROOT in build_windows.bat.
2. Run build_windows.bat from this folder.
3. The executable and deployed Qt runtime will be in build\Release.

GitHub Actions:
Upload this folder to a repository and run the workflow "Build PSVMS for Windows" from Actions. Download the PSVMS-windows artifact after the job completes.

Note: this environment does not contain Qt 6 or FFmpeg development packages, so a local compile/test could not be performed here.
