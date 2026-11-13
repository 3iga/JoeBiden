@echo off
setlocal
cd /d "%~dp0"
call build.bat
if errorlevel 1 goto failed
rem Photos are optional for startup, but required for final acceptance of the fish guide.
if not exist "assets\fish\photos_complete.marker" (
    call setup_assets.bat
    if errorlevel 1 echo WARNING: Photo setup incomplete. See docs\DEPENDENCIES_AND_ASSETS.md.
)
xcopy "assets\*" "bin\Debug\assets\" /E /I /Y >nul
pushd "bin\Debug"
LuminousReef.exe
set "RESULT=%ERRORLEVEL%"
popd
if not "%RESULT%"=="0" goto failed
exit /b 0
:failed
echo.
echo Launch/build failed. Keep this window open and read the first error.
pause
exit /b 1
