@echo off
setlocal
cd /d "%~dp0"
call build.bat
if errorlevel 1 goto failed
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
