@echo off
setlocal EnableExtensions EnableDelayedExpansion

set "CONFIG_QTDIR=C:\Qt\5.15.2\msvc2019"
set "CONFIG_VS_DIR=C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise"
set "CONFIG_BUILD_ARCH=x86"

set "SCRIPT_DIR=%~dp0"
set "PROJECT_FILE=%SCRIPT_DIR%mqtt-client.pro"
if not defined BUILD_DIR set "BUILD_DIR=%SCRIPT_DIR%build\windows-release"
if not defined BUILD_ARCH set "BUILD_ARCH=%CONFIG_BUILD_ARCH%"

call :resolve_qt
if errorlevel 1 exit /b 1

call :resolve_msvc
if errorlevel 1 exit /b 1

for %%I in ("%QMAKE%") do set "QT_BIN=%%~dpI"
set "WINDEPLOYQT=%QT_BIN%windeployqt.exe"
if not exist "%WINDEPLOYQT%" (
    echo [ERROR] windeployqt.exe not found: %WINDEPLOYQT%
    exit /b 1
)

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if errorlevel 1 (
    echo [ERROR] Failed to create build directory: %BUILD_DIR%
    exit /b 1
)

echo.
echo ========================================
echo Build mqtt-client (Windows %BUILD_ARCH%)
echo Qt %QT_VERSION%: %QMAKE%
echo Visual Studio %VS_VERSION%: %VS_DIR%
echo Build directory: %BUILD_DIR%
echo ========================================
echo.

pushd "%BUILD_DIR%"
if errorlevel 1 (
    echo [ERROR] Failed to enter build directory: %BUILD_DIR%
    exit /b 1
)

"%QMAKE%" "%PROJECT_FILE%" CONFIG+=release
if errorlevel 1 goto :build_failed

nmake /NOLOGO release
if errorlevel 1 goto :build_failed

"%WINDEPLOYQT%" --release "%BUILD_DIR%\release\mqtt-client.exe"
if errorlevel 1 goto :build_failed

popd
echo.
echo ========================================
echo Done. Run: %BUILD_DIR%\release\mqtt-client.exe
echo ========================================
exit /b 0

:resolve_qt
if not "%CONFIG_QTDIR%"=="" (
    set "QMAKE=%CONFIG_QTDIR%\bin\qmake.exe"
    if not exist "!QMAKE!" (
        echo [ERROR] Configured qmake.exe not found: !QMAKE!
        exit /b 1
    )
    call :read_qt_version "!QMAKE!"
    set "QT_VERSION=!QT_DETECTED_VERSION!"
    echo [Qt] Using configured Qt !QT_VERSION!: !QMAKE!
    exit /b 0
)

set /a QT_COUNT=0
if defined QTDIR call :add_qmake "%QTDIR%\bin\qmake.exe"
if defined QMAKE call :add_qmake "%QMAKE%"
for /f "delims=" %%I in ('where qmake.exe 2^>nul') do call :add_qmake "%%I"
if exist "C:\Qt" for /d %%V in ("C:\Qt\*") do for /d %%K in ("%%~fV\*") do call :add_qmake "%%~fK\bin\qmake.exe"
if exist "%USERPROFILE%\Qt" for /d %%V in ("%USERPROFILE%\Qt\*") do for /d %%K in ("%%~fV\*") do call :add_qmake "%%~fK\bin\qmake.exe"

if !QT_COUNT! EQU 0 (
    echo [ERROR] No Qt installation found in QTDIR, PATH, C:\Qt or %USERPROFILE%\Qt.
    exit /b 1
)

echo [Qt] Found installations:
for /l %%I in (1,1,!QT_COUNT!) do echo   [%%I] Qt !QT_VERSION_%%I! - !QT_PATH_%%I!

if not defined QT_CHOICE set /p "QT_CHOICE=Select Qt [1]: "
if not defined QT_CHOICE set "QT_CHOICE=1"

set "QMAKE="
for /l %%I in (1,1,!QT_COUNT!) do (
    if "!QT_CHOICE!"=="%%I" (
        set "QMAKE=!QT_PATH_%%I!"
        set "QT_VERSION=!QT_VERSION_%%I!"
    )
)
if not defined QMAKE (
    echo [ERROR] Invalid Qt selection: !QT_CHOICE!
    exit /b 1
)
echo [Qt] Selected Qt !QT_VERSION!: !QMAKE!
exit /b 0

:add_qmake
if "%~1"=="" exit /b 0
for %%P in ("%~1") do set "QT_CANDIDATE=%%~fP"
if not exist "!QT_CANDIDATE!" exit /b 0

for /l %%I in (1,1,!QT_COUNT!) do if /I "!QT_PATH_%%I!"=="!QT_CANDIDATE!" exit /b 0

call :read_qt_version "!QT_CANDIDATE!"
set /a QT_COUNT+=1
set "QT_PATH_!QT_COUNT!=!QT_CANDIDATE!"
set "QT_VERSION_!QT_COUNT!=!QT_DETECTED_VERSION!"
exit /b 0

:read_qt_version
set "QT_DETECTED_VERSION=unknown"
for /f "usebackq delims=" %%V in (`"%~1" -query QT_VERSION 2^>nul`) do set "QT_DETECTED_VERSION=%%V"
exit /b 0

:resolve_msvc
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not "%CONFIG_VS_DIR%"=="" (
    set "VS_DIR=%CONFIG_VS_DIR%"
    if not exist "!VS_DIR!\Common7\Tools\VsDevCmd.bat" (
        echo [ERROR] Configured Visual Studio not found: !VS_DIR!
        exit /b 1
    )
    set "VS_VERSION=unknown"
    echo [MSVC] Using configured Visual Studio: !VS_DIR!
) else (
    if not exist "!VSWHERE!" (
        echo [ERROR] vswhere.exe not found. Install Visual Studio C++ tools or set CONFIG_VS_DIR.
        exit /b 1
    )

    set /a VS_COUNT=0
    for /f "usebackq delims=" %%I in (`"!VSWHERE!" -all -sort -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set /a VS_COUNT+=1
        set "VS_PATH_!VS_COUNT!=%%I"
    )
    set /a VS_VERSION_INDEX=0
    for /f "usebackq delims=" %%V in (`"!VSWHERE!" -all -sort -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property catalog_productDisplayVersion`) do (
        set /a VS_VERSION_INDEX+=1
        set "VS_VERSION_!VS_VERSION_INDEX!=%%V"
    )

    if !VS_COUNT! EQU 0 (
        echo [ERROR] No Visual Studio installation with C++ tools found.
        exit /b 1
    )

    echo [MSVC] Found installations:
    for /l %%I in (1,1,!VS_COUNT!) do echo   [%%I] Visual Studio !VS_VERSION_%%I! - !VS_PATH_%%I!

    if not defined VS_CHOICE set /p "VS_CHOICE=Select Visual Studio [1]: "
    if not defined VS_CHOICE set "VS_CHOICE=1"

    set "VS_DIR="
    for /l %%I in (1,1,!VS_COUNT!) do (
        if "!VS_CHOICE!"=="%%I" (
            set "VS_DIR=!VS_PATH_%%I!"
            set "VS_VERSION=!VS_VERSION_%%I!"
        )
    )
    if not defined VS_DIR (
        echo [ERROR] Invalid Visual Studio selection: !VS_CHOICE!
        exit /b 1
    )
    echo [MSVC] Selected Visual Studio !VS_VERSION!: !VS_DIR!
)

call "!VS_DIR!\Common7\Tools\VsDevCmd.bat" -arch=%BUILD_ARCH%
if errorlevel 1 (
    echo [ERROR] Failed to initialize Visual Studio: !VS_DIR!
    exit /b 1
)
if defined VSCMD_VER set "VS_VERSION=!VSCMD_VER!"
if "!VS_VERSION!"=="unknown" if defined VisualStudioVersion set "VS_VERSION=!VisualStudioVersion!"
echo [MSVC] Active Visual Studio !VS_VERSION!: !VS_DIR!

where nmake.exe >nul 2>&1
if errorlevel 1 (
    echo [ERROR] nmake.exe not found after initializing Visual Studio.
    exit /b 1
)
exit /b 0

:build_failed
set "BUILD_ERROR=%ERRORLEVEL%"
popd
echo [ERROR] Build or deployment failed with exit code %BUILD_ERROR%.
exit /b %BUILD_ERROR%
