#include "hooks.h"
#include "features.h"
#include "game.h"
#include "menu.h"
#include <windows.h>
#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <MinHook.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#ifdef _MSC_VER
#include <intrin.h>
#define RETURN_ADDRESS() _ReturnAddress()
#else
#define RETURN_ADDRESS() __builtin_return_address(0)
#endif

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

namespace hooks{
  typedef HRESULT(__stdcall* present_fn)(IDXGISwapChain*, UINT, UINT);
  typedef HRESULT(__stdcall* resize_fn)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
  typedef BOOL(WINAPI* set_cursor_pos_fn)(int, int);
  typedef BOOL(WINAPI* get_cursor_pos_fn)(LPPOINT);
  typedef SHORT(WINAPI* get_key_state_fn)(int);
  typedef BOOL(WINAPI* get_keyboard_state_fn)(PBYTE);
  typedef UINT(WINAPI* get_raw_input_data_fn)(HRAWINPUT, UINT, LPVOID, PUINT, UINT);
  typedef UINT(WINAPI* get_raw_input_buffer_fn)(PRAWINPUT, PUINT, UINT);
  typedef HRESULT(__stdcall* di_create_fn)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
  typedef HRESULT(__stdcall* di_state_fn)(void*, DWORD, LPVOID);
  typedef HRESULT(__stdcall* di_data_fn)(void*, DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD);
  typedef void(*timer_update_fn)();

  static present_fn        o_present;
  static resize_fn         o_resize;
  static set_cursor_pos_fn       o_set_cursor_pos;
  static get_cursor_pos_fn       o_get_cursor_pos;
  static get_key_state_fn        o_get_async_key_state;
  static get_key_state_fn        o_get_key_state;
  static get_keyboard_state_fn   o_get_keyboard_state;
  static get_raw_input_data_fn   o_get_raw_input_data;
  static get_raw_input_buffer_fn o_get_raw_input_buffer;
  static di_state_fn             o_di_state[2];
  static di_data_fn              o_di_data[2];
  static timer_update_fn   o_timer_update;
  static WNDPROC           o_wnd_proc;

  static HWND                    window;
  static IDXGISwapChain*         swap_chain;
  static ID3D11Device*           device;
  static ID3D11DeviceContext*    context;
  static ID3D11RenderTargetView* rtv;

  static uptr  self_lo, self_hi;
  static POINT game_cursor;
  static bool  game_cursor_set;

  static bool blocked(void* caller){
    return menu::open && ((uptr)caller < self_lo || (uptr)caller >= self_hi);
  }

  static i32 bind_key(UINT msg, WPARAM wparam){
    switch(msg){
      case WM_KEYDOWN:
      case WM_SYSKEYDOWN:   return (i32)wparam;
      case WM_MBUTTONDOWN:  return VK_MBUTTON;
      case WM_XBUTTONDOWN:  return GET_XBUTTON_WPARAM(wparam) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2;
    }

    return 0;
  }

  static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
    if(msg == WM_KEYDOWN && wparam == VK_INSERT){
      if(!(lparam & (1 << 30))){
        menu::open      = !menu::open;
        game_cursor_set = false;
      }
      return 0;
    }

    if(menu::binding){
      if(i32 vk = bind_key(msg, wparam)){
        if(vk != VK_ESCAPE)
          features::cfg.fly_key = vk;
        menu::binding = false;
        return 0;
      }
    }

    if(menu::open){
      ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);

      if(msg == WM_INPUT)
        return DefWindowProcW(hwnd, msg, wparam, lparam);

      bool key   = msg >= WM_KEYFIRST && msg <= WM_KEYLAST && msg != WM_KEYUP && msg != WM_SYSKEYUP;
      bool touch = msg >= 0x240 && msg <= 0x24F; // WM_TOUCH, WM_POINTER*
      bool mouse = (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST) || touch;
      if(key || mouse)
        return 0;
    }

    return CallWindowProcW(o_wnd_proc, hwnd, msg, wparam, lparam);
  }

  static void release_device(){
    if(rtv){ rtv->Release(); rtv = nullptr; }
    if(context){ context->Release(); context = nullptr; }
    if(device){ device->Release(); device = nullptr; }
  }

  static bool attach(IDXGISwapChain* sc){
    if(sc == swap_chain)
      return true;

    DXGI_SWAP_CHAIN_DESC desc;
    char class_name[32] = {};
    if(FAILED(sc->GetDesc(&desc)) || !GetClassNameA(desc.OutputWindow, class_name, sizeof(class_name)) || strcmp(class_name, "UnrealWindow") != 0)
      return false;

    ID3D11Device* new_device = nullptr;
    if(FAILED(sc->GetDevice(__uuidof(ID3D11Device), (void**)&new_device)))
      return false;

    if(swap_chain != nullptr){
      ImGui_ImplDX11_Shutdown();
      release_device();
    }
    else{
      ImGui::CreateContext();
      menu::setup();
    }

    if(desc.OutputWindow != window){
      if(window != nullptr){
        SetWindowLongPtrW(window, GWLP_WNDPROC, (LONG_PTR)o_wnd_proc);
        ImGui_ImplWin32_Shutdown();
      }

      window     = desc.OutputWindow;
      ImGui_ImplWin32_Init(window);
      o_wnd_proc = (WNDPROC)SetWindowLongPtrW(window, GWLP_WNDPROC, (LONG_PTR)wnd_proc);
    }

    device = new_device;
    device->GetImmediateContext(&context);
    ImGui_ImplDX11_Init(device, context);
    swap_chain = sc;
    return true;
  }

  static HRESULT __stdcall present_hook(IDXGISwapChain* sc, UINT sync, UINT flags){
    if(!(flags & DXGI_PRESENT_TEST) && attach(sc)){
      if(rtv == nullptr){
        ID3D11Texture2D* back_buffer = nullptr;
        if(SUCCEEDED(sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer))){
          device->CreateRenderTargetView(back_buffer, nullptr, &rtv);
          back_buffer->Release();
        }
      }

      if(rtv != nullptr){
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        menu::draw();
        ImGui::Render();
        context->OMSetRenderTargets(1, &rtv, nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
      }
    }

    return o_present(sc, sync, flags);
  }

  static HRESULT __stdcall resize_hook(IDXGISwapChain* sc, UINT count, UINT width, UINT height, DXGI_FORMAT format, UINT flags){
    if(sc == swap_chain && rtv != nullptr){
      rtv->Release();
      rtv = nullptr;
    }

    return o_resize(sc, count, width, height, format, flags);
  }

  // game forces the cursor in the center with every frame, shitty remember cursor system
  static BOOL WINAPI set_cursor_pos_hook(int x, int y){
    if(blocked(RETURN_ADDRESS())){
      game_cursor     = { x, y };
      game_cursor_set = true;
      return TRUE;
    }

    return o_set_cursor_pos(x, y);
  }

  static BOOL WINAPI get_cursor_pos_hook(LPPOINT point){
    if(point != nullptr && blocked(RETURN_ADDRESS())){
      if(!game_cursor_set){
        o_get_cursor_pos(&game_cursor);
        game_cursor_set = true;
      }

      *point = game_cursor;
      return TRUE;
    }

    return o_get_cursor_pos(point);
  }

  static SHORT WINAPI get_async_key_state_hook(int vk){
    if(blocked(RETURN_ADDRESS()))
      return 0;

    return o_get_async_key_state(vk);
  }

  static SHORT WINAPI get_key_state_hook(int vk){
    if(blocked(RETURN_ADDRESS()))
      return 0;

    return o_get_key_state(vk);
  }

  static BOOL WINAPI get_keyboard_state_hook(PBYTE keys){
    BOOL result = o_get_keyboard_state(keys);
    if(result && blocked(RETURN_ADDRESS()))
      memset(keys, 0, 256);

    return result;
  }

  static void clear_raw(RAWINPUT* input){
    if(input->header.dwType == RIM_TYPEMOUSE)
      memset(&input->data.mouse, 0, sizeof(input->data.mouse));
    else if(input->header.dwType == RIM_TYPEKEYBOARD)
      memset(&input->data.keyboard, 0, sizeof(input->data.keyboard));
  }

  static UINT WINAPI get_raw_input_data_hook(HRAWINPUT handle, UINT command, LPVOID data, PUINT size, UINT header_size){
    UINT result = o_get_raw_input_data(handle, command, data, size, header_size);
    if(command == RID_INPUT && data != nullptr && result != (UINT)-1 && blocked(RETURN_ADDRESS()))
      clear_raw((RAWINPUT*)data);

    return result;
  }

  static UINT WINAPI get_raw_input_buffer_hook(PRAWINPUT data, PUINT size, UINT header_size){
    UINT result = o_get_raw_input_buffer(data, size, header_size);
    if(data != nullptr && result != (UINT)-1 && blocked(RETURN_ADDRESS())){
      RAWINPUT* input = data;
      for(UINT i = 0; i < result; i++){
        clear_raw(input);
        input = (RAWINPUT*)(((uptr)input + input->header.dwSize + 7) & ~(uptr)7); // NEXTRAWINPUTBLOCK
      }
    }

    return result;
  }

  static void timer_update_hook(){
    o_timer_update();
    features::frame();
  }

  template<i32 n>
  static HRESULT __stdcall di_state_hook(void* device, DWORD size, LPVOID data){
    HRESULT result = o_di_state[n](device, size, data);
    if(SUCCEEDED(result) && data != nullptr && blocked(RETURN_ADDRESS()))
      memset(data, 0, size);

    return result;
  }

  template<i32 n>
  static HRESULT __stdcall di_data_hook(void* device, DWORD size, LPDIDEVICEOBJECTDATA data, LPDWORD count, DWORD flags){
    HRESULT result = o_di_data[n](device, size, data, count, flags);
    if(SUCCEEDED(result) && count != nullptr && blocked(RETURN_ADDRESS()))
      *count = 0;

    return result;
  }

  // only if the game uses DirectInput
  static void hook_dinput(){
    HMODULE dinput = GetModuleHandleW(L"dinput8.dll");
    if(dinput == nullptr)
      return;

    di_create_fn create = (di_create_fn)GetProcAddress(dinput, "DirectInput8Create");
    const IID* iids[2]  = { &IID_IDirectInput8W, &IID_IDirectInput8A };
    void* state_hooks[2] = { (void*)di_state_hook<0>, (void*)di_state_hook<1> };
    void* data_hooks[2]  = { (void*)di_data_hook<0>, (void*)di_data_hook<1> };

    for(i32 i = 0; create != nullptr && i < 2; i++){
      IDirectInput8W* di = nullptr;
      if(FAILED(create(GetModuleHandleW(nullptr), DIRECTINPUT_VERSION, *iids[i], (void**)&di, nullptr)))
        continue;

      IDirectInputDevice8W* mouse = nullptr;
      if(SUCCEEDED(di->CreateDevice(GUID_SysMouse, &mouse, nullptr))){
        void** vtbl = *(void***)mouse;
        MH_CreateHook(vtbl[9], state_hooks[i], (void**)&o_di_state[i]);
        MH_CreateHook(vtbl[10], data_hooks[i], (void**)&o_di_data[i]);
        mouse->Release();
      }
      di->Release();
    }
  }

  static bool swap_chain_vtable(void*& present, void*& resize){
    WNDCLASSEXW wc   = { sizeof(wc) };
    wc.lpfnWndProc   = DefWindowProcW;
    wc.hInstance     = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"meowhack_dummy";
    RegisterClassExW(&wc);
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW, 0, 0, 64, 64, nullptr, nullptr, wc.hInstance, nullptr);

    DXGI_SWAP_CHAIN_DESC desc = {};
    desc.BufferCount       = 1;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow      = hwnd;
    desc.SampleDesc.Count  = 1;
    desc.Windowed          = TRUE;
    desc.SwapEffect        = DXGI_SWAP_EFFECT_DISCARD;

    IDXGISwapChain*      sc  = nullptr;
    ID3D11Device*        dev = nullptr;
    ID3D11DeviceContext* ctx = nullptr;
    bool ok = SUCCEEDED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &desc, &sc, &dev, nullptr, &ctx));
    if(ok){
      void** vtbl = *(void***)sc;
      present = vtbl[8];
      resize  = vtbl[13];
      ctx->Release();
      dev->Release();
      sc->Release();
    }

    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return ok;
  }

  bool init(){
    void* present = nullptr;
    void* resize  = nullptr;
    if(!swap_chain_vtable(present, resize) || MH_Initialize() != MH_OK)
      return false;

    HMODULE self = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)&init, &self);
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)((u8*)self + ((IMAGE_DOS_HEADER*)self)->e_lfanew);
    self_lo = (uptr)self;
    self_hi = self_lo + nt->OptionalHeader.SizeOfImage;

    MH_CreateHook(present, (void*)present_hook, (void**)&o_present);
    MH_CreateHook(resize, (void*)resize_hook, (void**)&o_resize);

    // block all input from game
    MH_CreateHookApi(L"user32", "SetCursorPos", (void*)set_cursor_pos_hook, (void**)&o_set_cursor_pos);
    MH_CreateHookApi(L"user32", "GetCursorPos", (void*)get_cursor_pos_hook, (void**)&o_get_cursor_pos);
    MH_CreateHookApi(L"user32", "GetAsyncKeyState", (void*)get_async_key_state_hook, (void**)&o_get_async_key_state);
    MH_CreateHookApi(L"user32", "GetKeyState", (void*)get_key_state_hook, (void**)&o_get_key_state);
    MH_CreateHookApi(L"user32", "GetKeyboardState", (void*)get_keyboard_state_hook, (void**)&o_get_keyboard_state);
    MH_CreateHookApi(L"user32", "GetRawInputData", (void*)get_raw_input_data_hook, (void**)&o_get_raw_input_data);
    MH_CreateHookApi(L"user32", "GetRawInputBuffer", (void*)get_raw_input_buffer_hook, (void**)&o_get_raw_input_buffer);
    hook_dinput();
    if(game::ok)
      MH_CreateHook((void*)(game::base + game::rva::timer_update), (void*)timer_update_hook, (void**)&o_timer_update);

    return MH_EnableHook(MH_ALL_HOOKS) == MH_OK;
  }

  void unload(){
    menu::open = false;
    MH_DisableHook(MH_ALL_HOOKS);
    if(window != nullptr)
      SetWindowLongPtrW(window, GWLP_WNDPROC, (LONG_PTR)o_wnd_proc);

    Sleep(200);

    if(swap_chain != nullptr){
      ImGui_ImplDX11_Shutdown();
      ImGui_ImplWin32_Shutdown();
      ImGui::DestroyContext();
    }

    release_device();
    MH_Uninitialize();
  }
}
