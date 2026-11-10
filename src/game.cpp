#include "game.h"
#include <windows.h>
#include <cstring>

namespace game{
  uptr base = 0;
  bool ok   = false;

  struct s_anchor{
    uptr       rva;
    const char bytes[24];
    u32        size;
  };

  static const s_anchor anchors[] = {
    { 0x1186920, "\x48\x83\xEC\x58\x48\x8B\x0D\x9D\xB2\x0B\x04\x48\x85\xC9\x0F\x84", 16 },
    { 0x107E950, "\x40\x56\x57\x41\x57\x48\x83\xEC\x40\x0F\xB6\x05\x96\xD3\x1A\x04", 16 },
    { 0x112CE10, "\x8B\x05\x42\xD6\x10\x04\x44\x0F\xB6\x15\xF8\xFA\x0E\x04\x44\x0F", 16 },
    { 0x107E170, "\x0F\xB6\x15\x7F\xDB\x1A\x04\x4C\x8D\x05\xA2\xD5\x36\x04\x48\x69\xCA\xC0\x01\x00\x00", 21 },
  };

  bool init(){
    base = (uptr)GetModuleHandleW(nullptr);
    ok   = true;
    for(const s_anchor& a : anchors){
      if(memcmp((void*)(base + a.rva), a.bytes, a.size) != 0)
        ok = false;
    }

    return ok;
  }

  u8* player_info(){
    u8 index = at<u8>(rva::player_in_focus);
    if(index > 1)
      index = 0;

    return (u8*)(base + rva::players + index * off::info_stride);
  }

  void* player_ped(){
    return *(void**)player_info();
  }

  void* player_vehicle(void* ped){
    if(ped == nullptr || !(field<u32>(ped, off::ped_flags) & 0x100))
      return nullptr;

    return field<void*>(ped, off::ped_vehicle);
  }

  f32* pos(void* entity){
    void* matrix = field<void*>(entity, off::entity_matrix);
    if(matrix != nullptr)
      return &field<f32>(matrix, off::matrix_pos);

    return &field<f32>(entity, off::entity_placement);
  }

  f32* camera_forward(){
    void* matrix = at<void*>(rva::camera_matrix);
    if(matrix == nullptr)
      return nullptr;

    return &field<f32>(matrix, off::matrix_forward);
  }

  void stop(void* entity){
    f32* speed = &field<f32>(entity, off::move_speed);
    for(i32 i = 0; i < 6; i++)
      speed[i] = 0.f;
  }

  void teleport(void* entity, f32 x, f32 y, f32 z){
    f32 dest[3] = { x, y, z };
    void** vtbl = *(void***)entity;
    ((void(*)(void*, f32*, bool))vtbl[off::vtbl_teleport / 8])(entity, dest, false);
  }

  f32 ground_z(f32 x, f32 y, bool* found){
    *found = false;
    return fn<f32(*)(f32, f32, f32, bool*, void*, void*)>(rva::find_ground_z)(x, y, 1000.f, found, nullptr, nullptr);
  }

  bool waypoint(f32& x, f32& y){
    for(i32 i = 0; i < 250; i++){
      u8* trace = (u8*)(base + rva::radar_trace + i * 0x30);
      if((trace[0x29] & 2) && (trace[0x2A] & 0x40)){
        x = *(f32*)(trace + 0x8);
        y = *(f32*)(trace + 0xC);
        return true;
      }
    }

    return false;
  }

  void set_wanted(void* ped, i32 stars){
    static const i32 chaos[7] = { 0, 70, 200, 570, 1220, 2420, 4620 };

    void* data = field<void*>(ped, off::ped_data);
    if(data == nullptr)
      return;

    void* wanted = *(void**)data;
    if(wanted == nullptr)
      return;

    stars = stars < 0 ? 0 : stars > 6 ? 6 : stars;
    field<i32>(wanted, off::wanted_chaos) = chaos[stars];
    field<i32>(wanted, off::wanted_level) = stars;
  }

  f32 time_step(){
    f32 step = at<f32>(rva::time_step);
    return step < 0.01f ? 0.01f : step > 3.f ? 3.f : step;
  }
}
