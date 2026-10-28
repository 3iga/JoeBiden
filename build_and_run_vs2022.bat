@echo off
setlocal
cd /d "%~dp0"
where cmake >nul 2>nul
if errorlevel 1 (
  echo CMake was not found. Open the x64 Native Tools Command Prompt for VS 2022,
  echo then run this file again. Install Desktop development with C++ and CMake tools.
  pause
  exit /b 1
)
cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64
if errorlevel 1 goto failed
cmake --build build-vs --config Release --parallel
if errorlevel 1 goto failed
ctest --test-dir build-vs -C Release --output-on-failure
if errorlevel 1 goto failed
pushd build-vs\Release
LuminousReef.exe
set RESULT=%ERRORLEVEL%
popd
if not "%RESULT%"=="0" goto failed
exit /b 0
:failed
echo.
echo Build, tests or launch failed. Read the FIRST error above.
pause
exit /b 1
