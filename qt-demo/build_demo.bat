@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x86
set PATH=C:\Qt\5.15.2\msvc2019\bin;%PATH%
cd /d D:\Code\GitHub\qmqtt\qt-demo
qmake mqtt-client.pro
if errorlevel 1 exit /b 1
nmake
if errorlevel 1 exit /b 1
echo Build OK
