#pragma once
#include "types.h"

namespace game{
  namespace rva{
    constexpr uptr timer_update    = 0x1186920; // CTimer::Update
    constexpr uptr vehicle_cheat   = 0x107E950; // CVehicle* (int model)
    constexpr uptr set_clock       = 0x112CE10; // (u8 h, u8 m, u8 day)
    constexpr uptr find_ground_z   = 0x1191AC0; // CWorld::FindGroundZFor3DCoord
    constexpr uptr request_model   = 0x1288560; // CStreaming::RequestModel
    constexpr uptr load_requested  = 0x128A360; // CStreaming::LoadAllRequestedModels
    constexpr uptr give_weapon     = 0x1222990; // CPed::GiveWeapon

    constexpr uptr player_in_focus = 0x522BCF6; // u8
    constexpr uptr players         = 0x53EB720; // CPlayerInfo[2]
    constexpr uptr time_scale      = 0x523A2DC; // f32
    constexpr uptr time_step       = 0x523A2D8; // f32
    constexpr uptr cheat_table     = 0x43064C8; // void(*)()[]
    constexpr uptr cheat_active    = 0x51BD0BB; // u8[]
    constexpr uptr clock_hours     = 0x521C917; // u8
    constexpr uptr clock_minutes   = 0x521C923; // u8
    constexpr uptr clock_day       = 0x521C922; // u8
    constexpr uptr weather_old     = 0x5300FEC; // i16
    constexpr uptr weather_new     = 0x5300FF0; // i16
    constexpr uptr weather_interp  = 0x5300FF4; // f32
    constexpr uptr weather_forced  = 0x5301008; // i16
    constexpr uptr camera_matrix   = 0x53E23E8; // CMatrix*
    constexpr uptr radar_trace     = 0x542A7B0; // tRadarTrace[250]
  }

  namespace off{
    constexpr uptr info_stride      = 0x1C0;
    constexpr uptr info_money       = 0xE8;
    constexpr uptr info_no_tired    = 0x180;
    constexpr uptr info_fast_reload = 0x181;
    constexpr uptr info_fireproof   = 0x182;
    constexpr uptr info_max_health  = 0x183;
    constexpr uptr info_max_armor   = 0x184;

    constexpr uptr entity_placement = 0x8;
    constexpr uptr entity_matrix    = 0x18;
    constexpr uptr matrix_forward   = 0x10;
    constexpr uptr matrix_pos       = 0x30;
    constexpr uptr physical_flags   = 0x78;
    constexpr uptr move_speed       = 0x7C;
    constexpr uptr vtbl_teleport    = 0x70;

    constexpr uptr ped_data         = 0x5F0;
    constexpr uptr ped_flags        = 0x634;
    constexpr uptr ped_health       = 0x76C;
    constexpr uptr ped_armor        = 0x774;
    constexpr uptr ped_vehicle      = 0x7C8;
    constexpr uptr ped_weapons      = 0x7F0;
    constexpr uptr weapon_stride    = 0x28;
    constexpr uptr weapon_total     = 0xC;

    constexpr uptr wanted_chaos     = 0x0;
    constexpr uptr wanted_level     = 0x2C;

    constexpr uptr vehicle_health   = 0x79C;
    constexpr uptr vtbl_fix         = 0x1C8;

    constexpr u32 bit_gravity    = 1u << 1;
    constexpr u32 bit_collidable = 1u << 3;
    constexpr u32 proof_bits     = (1u << 18) | (1u << 19) | (1u << 20) | (1u << 21) | (1u << 23);
  }

  extern uptr base;
  extern bool ok;

  bool init();

  template<typename T> inline T& at(uptr rva){ return *(T*)(base + rva); }
  template<typename T> inline T  fn(uptr rva){ return (T)(base + rva); }
  template<typename T> inline T& field(void* p, uptr off){ return *(T*)((u8*)p + off); }

  u8*   player_info();
  void* player_ped();
  void* player_vehicle(void* ped);
  f32*  pos(void* entity);
  f32*  camera_forward();
  void  stop(void* entity);
  void  teleport(void* entity, f32 x, f32 y, f32 z);
  f32   ground_z(f32 x, f32 y, bool* found);
  bool  waypoint(f32& x, f32& y);
  void  set_wanted(void* ped, i32 stars);
  f32   time_step();
}
