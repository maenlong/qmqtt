@echo off
setlocal enabledelayedexpansion

:: ======================== CONFIG ========================
set QTDIR=C:\Qt\5.15.2\msvc2019
set VS2017_DIR=C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise
:: ========================================================

cd /d "%~dp0" || (echo [ERROR] Failed to cd to script dir & pause & exit /b 1)

where nmake >nul 2>&1
if %errorlevel% neq 0 (
    if exist "!VS2017_DIR!\VC\Auxiliary\Build\vcvarsall.bat" (
        call "!VS2017_DIR!\VC\Auxiliary\Build\vcvarsall.bat" x86
    ) else (
        echo [ERROR] VS2017 not found
        pause
        exit /b 1
    )
)

set PATH=%QTDIR%\bin;%PATH%

echo ========================================
echo Build mqtt-client (qmqtt demo)
echo ========================================
echo.

if exist release rmdir /s /q release

qmake mqtt-client.pro CONFIG+=release
if %errorlevel% neq 0 ( echo [ERROR] qmake failed & pause & exit /b 1 )

nmake
if %errorlevel% neq 0 ( echo [ERROR] nmake failed & pause & exit /b 1 )

windeployqt release\mqtt-client.exe

echo.
echo ========================================
echo Done! Run: release\mqtt-client.exe
echo ========================================
pause
