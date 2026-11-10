#include "features.h"
#include "game.h"
#include "menu.h"
#include <windows.h>
#include <cmath>
#include <mutex>
#include <vector>

using namespace game;

namespace features{
  s_config cfg;

  static std::mutex                         jobs_lock;
  static std::vector<std::function<void()>> jobs;

  static bool key_down(i32 vk){
    DWORD pid = 0;
    GetWindowThreadProcessId(GetForegroundWindow(), &pid);
    return pid == GetCurrentProcessId() && (GetAsyncKeyState(vk) & 0x8000);
  }

  void queue(std::function<void()> job){
    std::lock_guard<std::mutex> lock(jobs_lock);
    jobs.push_back(std::move(job));
  }

  static void* player_entity(){
    void* ped = player_ped();
    void* veh = player_vehicle(ped);
    return veh != nullptr ? veh : ped;
  }

  static f32 max_health(){
    u8 value = player_info()[off::info_max_health];
    return value > 100 ? (f32)value : 100.f;
  }

  static f32 max_armor(){
    u8 value = player_info()[off::info_max_armor];
    return value > 100 ? (f32)value : 100.f;
  }

  // fly
  static void* fly_entity = nullptr;
  static u32   fly_flags  = 0;
  static f32   fly_pos[3];

  static void fly_release(void* ped, void* veh){
    void* e = fly_entity;
    fly_entity = nullptr;

    // only touch if we're still the owner of the car
    if(e != ped && e != veh)
      return;

    field<u32>(e, off::physical_flags) = fly_flags;

    f32* p     = pos(e);
    bool found = false;
    f32  z     = ground_z(p[0], p[1], &found);
    if(found && z < p[2])
      game::teleport(e, p[0], p[1], z + 1.f);
  }

  static void fly_frame(void* ped, void* veh){
    void* e = veh != nullptr ? veh : ped;

    if(fly_entity != nullptr && (!cfg.fly || fly_entity != e))
      fly_release(ped, veh);

    if(!cfg.fly)
      return;

    f32* p = pos(e);
    if(fly_entity == nullptr){
      fly_entity = e;
      fly_flags  = field<u32>(e, off::physical_flags);
      for(i32 i = 0; i < 3; i++)
        fly_pos[i] = p[i];
    }

    field<u32>(e, off::physical_flags) &= ~(off::bit_gravity | off::bit_collidable);

    if(!menu::open){
      f32* cam = camera_forward();
      f32  fwd[3] = { 0.f, 1.f, 0.f };
      if(cam != nullptr){
        for(i32 i = 0; i < 3; i++)
          fwd[i] = cam[i];
      }

      f32 flat    = sqrtf(fwd[0] * fwd[0] + fwd[1] * fwd[1]);
      f32 right_x = flat > 0.01f ? fwd[1] / flat : 1.f;
      f32 right_y = flat > 0.01f ? -fwd[0] / flat : 0.f;

      f32 step = 0.6f * cfg.fly_speed * time_step();
      if(key_down(VK_SHIFT))
        step *= 4.f;

      f32 move[3] = {};
      if(key_down('W')) for(i32 i = 0; i < 3; i++) move[i] += fwd[i];
      if(key_down('S')) for(i32 i = 0; i < 3; i++) move[i] -= fwd[i];
      if(key_down('D')){ move[0] += right_x; move[1] += right_y; }
      if(key_down('A')){ move[0] -= right_x; move[1] -= right_y; }
      if(key_down(VK_SPACE))   move[2] += 1.f;
      if(key_down(VK_CONTROL)) move[2] -= 1.f;

      for(i32 i = 0; i < 3; i++)
        fly_pos[i] += move[i] * step;
    }

    for(i32 i = 0; i < 3; i++)
      p[i] = fly_pos[i];

    stop(e);
  }

  // waypoints
  static bool wp_pending = false;
  static f32  wp_x, wp_y;
  static i32  wp_frames;

  static void waypoint_frame(void* e){
    if(!wp_pending)
      return;

    bool found = false;
    f32  z     = ground_z(wp_x, wp_y, &found);
    if(found || ++wp_frames > 400){
      game::teleport(e, wp_x, wp_y, found ? z + 1.f : 20.f);
      wp_pending = false;
      return;
    }

    f32* p = pos(e);
    p[0] = wp_x;
    p[1] = wp_y;
    p[2] = 300.f;
    stop(e);
  }

  // vehicle godmode
  static void* proof_vehicle = nullptr;
  static u32   proof_saved   = 0;

  static void vehicle_frame(void* veh){
    if(proof_vehicle != nullptr && (proof_vehicle != veh || !cfg.vehicle_god)){
      if(proof_vehicle == veh){
        u32& flags = field<u32>(veh, off::physical_flags);
        flags = (flags & ~off::proof_bits) | proof_saved;
      }
      proof_vehicle = nullptr;
    }

    if(veh == nullptr)
      return;

    f32& health = field<f32>(veh, off::vehicle_health);
    if(cfg.vehicle_god){
      u32& flags = field<u32>(veh, off::physical_flags);
      if(proof_vehicle == nullptr){
        proof_vehicle = veh;
        proof_saved   = flags & off::proof_bits;
      }
      flags |= off::proof_bits;
    }

    if(cfg.vehicle_repair && health < 1000.f){
      void** vtbl = *(void***)veh;
      ((void(*)(void*))vtbl[off::vtbl_fix / 8])(veh);
    }

    if((cfg.vehicle_god || cfg.vehicle_repair) && health < 1000.f)
      health = 1000.f;
  }

  // player perks
  static void perk(uptr offset, bool on, bool& applied){
    u8& flag = player_info()[offset];
    if(on){
      flag    = 1;
      applied = true;
    }
    else if(applied){
      flag    = 0;
      applied = false;
    }
  }

  static void apply_weather(i32 id){
    at<i16>(rva::weather_old)    = (i16)id;
    at<i16>(rva::weather_new)    = (i16)id;
    at<i16>(rva::weather_forced) = (i16)id;
    at<f32>(rva::weather_interp) = 0.f;
  }

  static void world_frame(){
    if(cfg.freeze_time){
      at<u8>(rva::clock_hours)   = (u8)cfg.hour;
      at<u8>(rva::clock_minutes) = (u8)cfg.minute;
    }

    if(cfg.freeze_weather)
      apply_weather(cfg.weather);

    if(cfg.game_speed)
      at<f32>(rva::time_scale) = cfg.speed;
  }

  void frame(){
    std::vector<std::function<void()>> work;
    {
      std::lock_guard<std::mutex> lock(jobs_lock);
      work.swap(jobs);
    }

    void* ped = player_ped();
    if(ped == nullptr)
      return;

    for(auto& job : work)
      job();

    static bool fly_was_down = false;
    bool fly_down = key_down(cfg.fly_key);
    if(fly_down && !fly_was_down && !menu::open && !menu::binding)
      cfg.fly = !cfg.fly;
    fly_was_down = fly_down;

    void* veh = player_vehicle(ped);
    fly_frame(ped, veh);
    waypoint_frame(veh != nullptr ? veh : ped);
    vehicle_frame(veh);
    world_frame();

    static bool no_tired, fast_reload, fireproof;
    perk(off::info_no_tired, cfg.no_tired, no_tired);
    perk(off::info_fast_reload, cfg.fast_reload, fast_reload);
    perk(off::info_fireproof, cfg.fireproof, fireproof);

    f32& health = field<f32>(ped, off::ped_health);
    f32& armor  = field<f32>(ped, off::ped_armor);
    if(cfg.god && health < max_health())
      health = max_health();

    if(cfg.armor && armor < max_armor())
      armor = max_armor();

    if(cfg.never_wanted)
      game::set_wanted(ped, 0);

    if(cfg.inf_ammo){
      for(i32 i = 0; i < 13; i++){
        u8*  slot  = (u8*)ped + off::ped_weapons + i * off::weapon_stride;
        i32& total = *(i32*)(slot + off::weapon_total);
        if(*(i32*)slot > 0 && total > 0 && total < 9000)
          total = 9999;
      }
    }
  }

  // teleports
  void teleport(f32 x, f32 y, f32 z){
    queue([=]{
      game::teleport(player_entity(), x, y, z);
      fly_pos[0] = x;
      fly_pos[1] = y;
      fly_pos[2] = z;
    });
  }

  void teleport_waypoint(){
    queue([]{
      f32 x, y;
      if(!waypoint(x, y))
        return;

      bool found = false;
      f32  z     = ground_z(x, y, &found);
      if(found){
        game::teleport(player_entity(), x, y, z + 1.f);
        return;
      }

      game::teleport(player_entity(), x, y, 300.f);
      wp_pending = true;
      wp_x       = x;
      wp_y       = y;
      wp_frames  = 0;
    });
  }

  static f32  slots[3][3];
  static bool slots_used[3];

  void save_slot(i32 slot){
    queue([=]{
      f32* p = pos(player_entity());
      for(i32 i = 0; i < 3; i++)
        slots[slot][i] = p[i];
      slots_used[slot] = true;
    });
  }

  void load_slot(i32 slot){
    if(slots_used[slot])
      teleport(slots[slot][0], slots[slot][1], slots[slot][2] + 0.5f);
  }

  bool slot_used(i32 slot){
    return slots_used[slot];
  }

  void spawn_vehicle(i32 model){
    queue([=]{ fn<void*(*)(i32)>(rva::vehicle_cheat)(model); });
  }

  void repair_vehicle(){
    queue([]{
      void* veh = player_vehicle(player_ped());
      if(veh == nullptr)
        return;

      void** vtbl = *(void***)veh;
      ((void(*)(void*))vtbl[off::vtbl_fix / 8])(veh);
      field<f32>(veh, off::vehicle_health) = 1000.f;
    });
  }

  void flip_vehicle(){
    queue([]{
      void* veh = player_vehicle(player_ped());
      if(veh == nullptr)
        return;

      f32* m = &field<f32>(field<void*>(veh, off::entity_matrix), 0);
      f32  x = m[4], y = m[5];
      f32  len = sqrtf(x * x + y * y);
      if(len < 0.01f){ x = 0.f; y = 1.f; len = 1.f; }
      x /= len;
      y /= len;

      m[0] = y;   m[1] = -x;  m[2]  = 0.f; // right
      m[4] = x;   m[5] = y;   m[6]  = 0.f; // forward
      m[8] = 0.f; m[9] = 0.f; m[10] = 1.f; // up
      m[14] += 1.f;
    });
  }

  void give_weapon(i32 type, i32 model, i32 ammo){
    queue([=]{
      auto request = fn<void(*)(i32, i32)>(rva::request_model);
      request(model, 2);
      if(type == 39)
        request(364, 2);

      fn<void(*)(bool)>(rva::load_requested)(false);
      fn<void(*)(void*, i32, i32)>(rva::give_weapon)(player_ped(), type, ammo);
    });
  }

  void run_cheat(i32 index){
    queue([=]{
      void(*cheat)() = ((void(**)())(base + rva::cheat_table))[index];
      if(cheat != nullptr)
        cheat();
      else
        at<u8>(rva::cheat_active + index) ^= 1;
    });
  }

  void refill(){
    queue([]{
      void* ped = player_ped();
      field<f32>(ped, off::ped_health) = max_health();
      field<f32>(ped, off::ped_armor)  = max_armor();
    });
  }

  void set_wanted(i32 stars){
    queue([=]{ game::set_wanted(player_ped(), stars); });
  }

  void add_money(i32 amount){
    queue([=]{ *(i32*)(player_info() + off::info_money) += amount; });
  }

  void set_weather(i32 id){
    queue([=]{ apply_weather(id); });
  }

  void set_time(i32 hour, i32 minute){
    queue([=]{ fn<void(*)(u8, u8, u8)>(rva::set_clock)((u8)hour, (u8)minute, at<u8>(rva::clock_day)); });
  }
}
