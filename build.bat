@echo off
setlocal EnableExtensions
set "ROOT=%~dp0"
cd /d "%ROOT%"
call "%ROOT%scripts\init_msvc.bat"
if errorlevel 1 goto failed
call "%ROOT%setup_dependencies.bat"
if errorlevel 1 goto failed
for %%D in ("obj\Debug\glfw" "obj\Debug\core" "obj\Debug\imgui" "obj\Debug\tests" "bin\Debug" "include\glfw\lib") do if not exist "%%~D" mkdir "%%~D"
set "COMMON=/nologo /Od /Zi /MTd /D_DEBUG /DNOMINMAX /DWIN32_LEAN_AND_MEAN /D_CRT_SECURE_NO_WARNINGS /utf-8"
set "CPP=%COMMON% /std:c++17 /EHs-c- /D_HAS_EXCEPTIONS=0"
set "INC=/I"%ROOT%include" /I"%ROOT%include\glfw\include" /I"%ROOT%include\imgui" /I"%ROOT%src""

echo [1/6] Compiling GLFW for Windows x64 Debug...
pushd "%ROOT%obj\Debug\glfw"
cl %COMMON% /W3 /TC /c /D_GLFW_WIN32 /I"%ROOT%include\glfw\include" /I"%ROOT%include\glfw\src" ^
    "%ROOT%include\glfw\src\context.c" ^
    "%ROOT%include\glfw\src\init.c" ^
    "%ROOT%include\glfw\src\input.c" ^
    "%ROOT%include\glfw\src\monitor.c" ^
    "%ROOT%include\glfw\src\platform.c" ^
    "%ROOT%include\glfw\src\vulkan.c" ^
    "%ROOT%include\glfw\src\window.c" ^
    "%ROOT%include\glfw\src\egl_context.c" ^
    "%ROOT%include\glfw\src\osmesa_context.c" ^
    "%ROOT%include\glfw\src\null_init.c" ^
    "%ROOT%include\glfw\src\null_monitor.c" ^
    "%ROOT%include\glfw\src\null_window.c" ^
    "%ROOT%include\glfw\src\null_joystick.c" ^
    "%ROOT%include\glfw\src\win32_init.c" ^
    "%ROOT%include\glfw\src\win32_joystick.c" ^
    "%ROOT%include\glfw\src\win32_monitor.c" ^
    "%ROOT%include\glfw\src\win32_window.c" ^
    "%ROOT%include\glfw\src\win32_module.c" ^
    "%ROOT%include\glfw\src\win32_time.c" ^
    "%ROOT%include\glfw\src\win32_thread.c" ^
    "%ROOT%include\glfw\src\wgl_context.c"
if errorlevel 1 goto failedpop
lib /nologo /OUT:"%ROOT%include\glfw\lib\glfw3_debug.lib" *.obj
if errorlevel 1 goto failedpop
popd

echo [2/6] Compiling GLAD and project code...
pushd "%ROOT%obj\Debug\core"
cl %COMMON% /W3 /TC /c /I"%ROOT%include" "%ROOT%src\glad_impl.c"
if errorlevel 1 goto failedpop
cl %CPP% /W4 /DGLFW_INCLUDE_NONE %INC% /c "%ROOT%src\*.cpp"
if errorlevel 1 goto failedpop
popd

echo [3/6] Compiling Dear ImGui...
pushd "%ROOT%obj\Debug\imgui"
cl %CPP% /W3 /DGLFW_INCLUDE_NONE %INC% /c ^
    "%ROOT%include\imgui\imgui.cpp" ^
    "%ROOT%include\imgui\imgui_draw.cpp" ^
    "%ROOT%include\imgui\imgui_tables.cpp" ^
    "%ROOT%include\imgui\imgui_widgets.cpp" ^
    "%ROOT%include\imgui\backends\imgui_impl_glfw.cpp" ^
    "%ROOT%include\imgui\backends\imgui_impl_opengl3.cpp"
if errorlevel 1 goto failedpop
popd

echo [4/6] Linking LuminousReef.exe...
link /nologo /DEBUG /INCREMENTAL:NO /SUBSYSTEM:CONSOLE /MACHINE:X64 ^
    /OUT:"%ROOT%bin\Debug\LuminousReef.exe" /PDB:"%ROOT%bin\Debug\LuminousReef.pdb" ^
    "%ROOT%obj\Debug\core\*.obj" "%ROOT%obj\Debug\imgui\*.obj" ^
    "%ROOT%include\glfw\lib\glfw3_debug.lib" opengl32.lib user32.lib gdi32.lib shell32.lib
if errorlevel 1 goto failed

echo [5/6] Building and running CPU tests...
pushd "%ROOT%obj\Debug\tests"
cl %CPP% /W4 %INC% "%ROOT%tests\cpu_tests.cpp" ^
    "%ROOT%obj\Debug\core\Geometry.obj" "%ROOT%obj\Debug\core\Scene.obj" ^
    "%ROOT%obj\Debug\core\Update.obj" "%ROOT%obj\Debug\core\Species.obj" ^
    /Fe:"%ROOT%bin\Debug\cpu_tests.exe" /link /DEBUG /INCREMENTAL:NO
if errorlevel 1 goto failedpop
popd
"%ROOT%bin\Debug\cpu_tests.exe"
if errorlevel 1 goto failed

echo [6/6] Copying shaders and assets beside executable...
xcopy "%ROOT%shaders\*" "%ROOT%bin\Debug\shaders\" /E /I /Y >nul
if errorlevel 1 goto failed
xcopy "%ROOT%assets\*" "%ROOT%bin\Debug\assets\" /E /I /Y >nul
if errorlevel 1 goto failed
echo SUCCESS: bin\Debug\LuminousReef.exe
exit /b 0
:failedpop
popd
:failed
echo.
echo BUILD FAILED. Read the FIRST error above. No old executable will be launched.
exit /b 1
