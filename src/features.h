#pragma once
#include "types.h"
#include <functional>

namespace features{
  struct s_config{
    bool god          = false;
    bool armor        = false;
    bool never_wanted = false;
    bool inf_ammo     = false;
    bool no_tired     = false;
    bool fast_reload  = false;
    bool fireproof    = false;

    bool fly       = false;
    i32  fly_key   = 0x75; // VK_F6, default binding F6
    f32  fly_speed = 1.f;

    bool vehicle_god    = false;
    bool vehicle_repair = false;

    bool freeze_time    = false;
    i32  hour           = 12;
    i32  minute         = 0;
    bool freeze_weather = false;
    i32  weather        = 1;
    bool game_speed     = false;
    f32  speed          = 1.f;
  };

  extern s_config cfg;

  void frame();

  // all features
  void queue(std::function<void()> job);

  void teleport(f32 x, f32 y, f32 z);
  void teleport_waypoint();
  void save_slot(i32 slot);
  void load_slot(i32 slot);
  bool slot_used(i32 slot);

  void spawn_vehicle(i32 model);
  void repair_vehicle();
  void flip_vehicle();

  void give_weapon(i32 type, i32 model, i32 ammo);
  void run_cheat(i32 index);

  void refill();
  void set_wanted(i32 stars);
  void add_money(i32 amount);
  void set_weather(i32 id);
  void set_time(i32 hour, i32 minute);
}
