@echo off
rem Keep environment variables in the caller. Do NOT use setlocal here.
if /I "%VSCMD_ARG_TGT_ARCH%"=="x64" (
    where cl.exe >nul 2>nul
    if not errorlevel 1 exit /b 0
)
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo ERROR: Visual Studio Installer / vswhere.exe not found.
    echo Install Visual Studio with Desktop development with C++ and a Windows SDK.
    exit /b 1
)
set "VSROOT="
for /f "usebackq tokens=*" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSROOT=%%I"
if not defined VSROOT (
    echo ERROR: No MSVC x64 tools found. Add Desktop development with C++.
    exit /b 1
)
call "%VSROOT%\VC\Auxiliary\Build\vcvarsall.bat" x64
if errorlevel 1 exit /b 1
where cl.exe >nul 2>nul
if errorlevel 1 exit /b 1
exit /b 0
