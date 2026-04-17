@REM Build for Visual Studio compiler. Run your copy of vcvars64.bat or vcvarsall.bat to setup command-line compiler.

@set OUT_EXE=8085sim
@set INCLUDES=/I.\thirdparty\imgui /I.\thirdparty\imgui\backends /I.\thirdparty\SDL3\windows\include /I.\thirdparty\log
@set SOURCES=.\src\main.cpp .\thirdparty\imgui\backends\imgui_impl_sdl3.cpp .\thirdparty\imgui\backends\imgui_impl_opengl3.cpp .\thirdparty\imgui\imgui*.cpp .\thirdparty\log\*.c
@set LIBS=/LIBPATH:.\thirdparty\SDL3\windows\lib SDL3.lib opengl32.lib shell32.lib

@set OUT_DIR=build
mkdir %OUT_DIR%
cl /nologo /Zi /MD /utf-8 %INCLUDES% %SOURCES% /Fe%OUT_DIR%/%OUT_EXE%.exe /Fo%OUT_DIR%/ /link %LIBS% /subsystem:console
copy .\thirdparty\SDL3\windows\lib\*.dll %OUT_DIR%

rem @set OUT_DIR=build
rem mkdir %OUT_DIR%
rem cl /nologo /Zi /MD /utf-8 /Ox /Oi %INCLUDES% %SOURCES% /Fe%OUT_DIR%/%OUT_EXE%.exe /Fo%OUT_DIR%/ /link %LIBS% /subsystem:console
rem copy .\thirdparty\SDL3\windows\lib\*.dll %OUT_DIR%
