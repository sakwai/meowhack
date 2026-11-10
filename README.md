# meowhack

Internal mod menu for GTA: San Andreas - Definitive Edition

Offsets/sigs in `src/game.h` are for game build **1.0.112.48699928**. On any other build the menu still opens, but it shows a warning and won't touch game memory.

## How to build?
1. Download [MSYS2](https://www.msys2.org/) and install it to `C:\msys64`.
2. Open the MSYS2 UCRT64 terminal from your start menu.
3. Run `pacman -Syuu` until `there is nothing to do.` is displayed.
4. Run `pacman -S --needed mingw-w64-ucrt-x86_64-gcc`.
5. Run `build_msys2.bat`. If MSYS2 is installed elsewhere, run `set MSYS2=D:\msys64` first. You have now built `bin\meowhack.dll`.

## How to run?
After you have built the source
1. Start the game in DX11 (default).
2. Inject `bin\meowhack.dll` into `SanAndreas.exe`.

## Keybinds
* `Insert` opens and closes the menu.
* `End` unloads the menu.
