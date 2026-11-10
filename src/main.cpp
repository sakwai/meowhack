#include "game.h"
#include "hooks.h"
#include <windows.h>

static bool end_pressed(){
  DWORD pid = 0;
  GetWindowThreadProcessId(GetForegroundWindow(), &pid);
  return pid == GetCurrentProcessId() && (GetAsyncKeyState(VK_END) & 0x8000);
}

static DWORD WINAPI main_thread(LPVOID module){
  game::init();

  if(hooks::init()){
    while(!end_pressed())
      Sleep(50);
  }

  hooks::unload();
  FreeLibraryAndExitThread((HMODULE)module, 0);
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID){
  if(reason == DLL_PROCESS_ATTACH){
    DisableThreadLibraryCalls(instance);
    if(HANDLE thread = CreateThread(nullptr, 0, main_thread, instance, 0, nullptr))
      CloseHandle(thread);
  }

  return TRUE;
}
