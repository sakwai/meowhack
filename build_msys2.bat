@echo off
REM Builds bin\meowhack.dll from src\ and vendor\ only, with MSYS2's UCRT64 gcc.
REM Needs: pacman -S mingw-w64-ucrt-x86_64-gcc
REM MSYS2 installed somewhere else? set MSYS2=D:\msys64 before running.
setlocal enabledelayedexpansion
cd /d "%~dp0"

if "%MSYS2%"=="" set MSYS2=C:\msys64
set PATH=%MSYS2%\ucrt64\bin;%PATH%
where g++ >nul 2>nul || (echo g++ not found in %MSYS2%\ucrt64\bin & exit /b 1)

if not exist build mkdir build
if not exist bin mkdir bin

REM minhook is C, so it gets compiled on its own first
for %%f in (vendor\minhook\buffer.c vendor\minhook\hook.c vendor\minhook\trampoline.c vendor\minhook\hde\hde64.c) do (
  gcc -c -O2 -Ivendor\minhook\include "%%f" -o "build\%%~nf.o" || exit /b 1
)

set files=
for %%f in (src\*.cpp vendor\imgui\*.cpp vendor\imgui\backends\*.cpp build\*.o) do set files=!files! "%%f"

g++ -std=c++20 -O2 -Wall -shared -DNOMINMAX ^
  -Ivendor\imgui -Ivendor\imgui\backends -Ivendor\minhook\include ^
  !files! -o bin\meowhack.dll ^
  -static -static-libgcc -static-libstdc++ ^
  -ld3d11 -ldxgi -ldxguid -ld3dcompiler -ldwmapi -lgdi32 || exit /b 1

rmdir /s /q build
echo Done: bin\meowhack.dll
endlocal
