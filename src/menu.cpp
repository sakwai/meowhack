#include "menu.h"
#include "features.h"
#include "game.h"
#include "tables.h"
#include <windows.h>
#include <imgui.h>
#include <cmath>
#include <cstdio>

using features::cfg;

namespace menu{
  bool open    = false;
  bool binding = false;

  void setup(){
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename  = nullptr;
    io.LogFilename  = nullptr;
    // custom imgui cursor
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    char path[MAX_PATH];
    GetWindowsDirectoryA(path, MAX_PATH);
    strcat_s(path, "\\Fonts\\segoeui.ttf");
    if(GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES || io.Fonts->AddFontFromFileTTF(path, 18.f) == nullptr)
      io.Fonts->AddFontDefault();

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.f;
    style.FrameRounding  = 3.f;
    style.GrabRounding   = 3.f;
    style.WindowPadding  = { 10.f, 10.f };
  }

  static void watermark(){
    ImDrawList* draw = ImGui::GetForegroundDrawList();
    const char* text = "Hello :)";

    f32 hue = fmodf((f32)ImGui::GetTime() * 0.25f, 1.f);
    f32 x   = 8.f;
    for(const char* c = text; *c; c++){
      char ch[2] = { *c, 0 };
      f32  r, g, b;
      ImGui::ColorConvertHSVtoRGB(hue, 1.f, 1.f, r, g, b);
      draw->AddText({ x + 1.f, 7.f }, IM_COL32(0, 0, 0, 200), ch);
      draw->AddText({ x, 6.f }, ImGui::ColorConvertFloat4ToU32({ r, g, b, 1.f }), ch);
      x  += ImGui::CalcTextSize(ch).x;
      hue = fmodf(hue + 0.05f, 1.f);
    }
  }

  static void key_name(i32 vk, char* out, i32 size){
    switch(vk){
      case VK_MBUTTON:  snprintf(out, size, "mouse 3"); return;
      case VK_XBUTTON1: snprintf(out, size, "mouse 4"); return;
      case VK_XBUTTON2: snprintf(out, size, "mouse 5"); return;
    }

    u32 scan = MapVirtualKeyA(vk, MAPVK_VK_TO_VSC);
    switch(vk){
      case VK_LEFT: case VK_RIGHT: case VK_UP: case VK_DOWN:
      case VK_PRIOR: case VK_NEXT: case VK_HOME: case VK_END:
      case VK_INSERT: case VK_DELETE: case VK_DIVIDE: case VK_NUMLOCK:
        scan |= 0x100;
    }

    if(!GetKeyNameTextA((LONG)(scan << 16), out, size))
      snprintf(out, size, "key %d", vk);
  }

  static void tab_player(){
    ImGui::SeparatorText("fly");
    ImGui::Checkbox("fly", &cfg.fly);
    ImGui::SameLine(200.f);

    char label[48] = "press a key";
    if(!binding)
      key_name(cfg.fly_key, label, sizeof(label));
    strcat_s(label, "###fly_key");
    if(ImGui::Button(label, { 120.f, 0.f }))
      binding = true;

    ImGui::SliderFloat("fly speed", &cfg.fly_speed, 0.1f, 10.f, "%.1f");

    ImGui::SeparatorText("player");
    ImGui::Checkbox("godmode", &cfg.god);
    ImGui::SameLine(200.f);
    ImGui::Checkbox("infinite armor", &cfg.armor);
    ImGui::Checkbox("infinite ammo", &cfg.inf_ammo);
    ImGui::SameLine(200.f);
    ImGui::Checkbox("never wanted", &cfg.never_wanted);
    ImGui::Checkbox("infinite sprint", &cfg.no_tired);
    ImGui::SameLine(200.f);
    ImGui::Checkbox("fast reload", &cfg.fast_reload);
    ImGui::Checkbox("fireproof", &cfg.fireproof);

    if(ImGui::Button("refill health + armor"))
      features::refill();

    static i32 stars = 0;
    ImGui::SliderInt("##stars", &stars, 0, 6, "%d stars");
    ImGui::SameLine();
    if(ImGui::Button("set wanted level"))
      features::set_wanted(stars);

    static i32 money = 100000;
    ImGui::InputInt("##money", &money, 1000, 100000);
    ImGui::SameLine();
    if(ImGui::Button("add money"))
      features::add_money(money);
  }

  static void tab_vehicle(){
    ImGui::Checkbox("vehicle god mode", &cfg.vehicle_god);
    ImGui::SameLine(200.f);
    ImGui::Checkbox("auto repair", &cfg.vehicle_repair);

    if(ImGui::Button("repair"))
      features::repair_vehicle();
    ImGui::SameLine();
    if(ImGui::Button("flip vehicle"))
      features::flip_vehicle();

    ImGui::SeparatorText("spawn");
    for(const sa::VehicleGroup& group : sa::kVehicleGroups){
      if(!ImGui::CollapsingHeader(group.name))
        continue;

      for(i32 i = 0; i < group.count; i++){
        ImGui::PushID(group.models[i].id);
        if(ImGui::Selectable(group.models[i].name))
          features::spawn_vehicle(group.models[i].id);
        ImGui::PopID();
      }
    }
  }

  static void tab_teleport(){
    if(ImGui::Button("teleport to waypoint"))
      features::teleport_waypoint();

    ImGui::SeparatorText("saved");
    for(i32 i = 0; i < 3; i++){
      ImGui::PushID(i);
      if(ImGui::Button("save"))
        features::save_slot(i);
      ImGui::SameLine();
      ImGui::BeginDisabled(!features::slot_used(i));
      if(ImGui::Button("teleport"))
        features::load_slot(i);
      ImGui::EndDisabled();
      ImGui::SameLine();
      ImGui::Text("slot %d", i + 1);
      ImGui::PopID();
    }

    ImGui::SeparatorText("places");
    for(const sa::Place& place : sa::kPlaces){
      if(ImGui::Selectable(place.name))
        features::teleport(place.x, place.y, place.z);
    }
  }

  static void tab_weapons(){
    for(const sa::WeaponGroup& group : sa::kWeaponGroups){
      if(!ImGui::CollapsingHeader(group.name))
        continue;

      for(i32 i = 0; i < group.count; i++){
        const sa::WeaponModel& weapon = group.weapons[i];
        ImGui::PushID(weapon.type);
        if(ImGui::Selectable(weapon.name))
          features::give_weapon(weapon.type, weapon.model, weapon.ammo);
        ImGui::PopID();
      }
    }
  }

  static void tab_world(){
    ImGui::SeparatorText("weather");
    ImGui::Combo("##weather", &cfg.weather, sa::kWeather, IM_ARRAYSIZE(sa::kWeather));
    ImGui::SameLine();
    if(ImGui::Button("set##weather"))
      features::set_weather(cfg.weather);
    ImGui::Checkbox("freeze weather", &cfg.freeze_weather);

    ImGui::SeparatorText("time");
    ImGui::SliderInt("hour", &cfg.hour, 0, 23);
    ImGui::SliderInt("minute", &cfg.minute, 0, 59);
    if(ImGui::Button("set##time"))
      features::set_time(cfg.hour, cfg.minute);
    ImGui::SameLine();
    ImGui::Checkbox("freeze time", &cfg.freeze_time);

    ImGui::SeparatorText("game speed");
    ImGui::Checkbox("custom speed", &cfg.game_speed);
    ImGui::SliderFloat("##speed", &cfg.speed, 0.1f, 3.f, "%.2fx");
  }

  static void cheat_group(const char* name, const sa::CheatInfo* cheats, i32 count){
    if(!ImGui::CollapsingHeader(name))
      return;

    for(i32 i = 0; i < count; i++){
      ImGui::PushID(cheats[i].index);
      if(ImGui::Selectable(cheats[i].name))
        features::run_cheat(cheats[i].index);
      ImGui::PopID();
    }
  }

  static void tab_cheats(){
    cheat_group("player", sa::kPlayerCheats, IM_ARRAYSIZE(sa::kPlayerCheats));
    cheat_group("stats", sa::kStatCheats, IM_ARRAYSIZE(sa::kStatCheats));
    cheat_group("weapons", sa::kWeaponCheats, IM_ARRAYSIZE(sa::kWeaponCheats));
    cheat_group("vehicles", sa::kVehicleCheats, IM_ARRAYSIZE(sa::kVehicleCheats));
    cheat_group("traffic", sa::kTrafficCheats, IM_ARRAYSIZE(sa::kTrafficCheats));
    cheat_group("peds", sa::kPedCheats, IM_ARRAYSIZE(sa::kPedCheats));
    cheat_group("world", sa::kWorldCheats, IM_ARRAYSIZE(sa::kWorldCheats));
  }

  void draw(){
    watermark();

    ImGui::GetIO().MouseDrawCursor = open;
    if(!open){
      binding = false;
      return;
    }

    ImGui::SetNextWindowSize({ 520.f, 480.f }, ImGuiCond_Once);
    ImGui::SetNextWindowPos({ 60.f, 60.f }, ImGuiCond_Once);
    ImGui::Begin("meowhack", &open, ImGuiWindowFlags_NoCollapse);

    if(!game::ok){
      ImGui::TextColored({ 1.f, 0.4f, 0.3f, 1.f }, "unsupported SanAndreas.exe build (version needed: 1.112)");
      ImGui::End();
      return;
    }

    if(ImGui::BeginTabBar("tabs")){
      struct{ const char* name; void(*draw)(); } tabs[] = {
        { "player",   tab_player   },
        { "vehicle",  tab_vehicle  },
        { "teleport", tab_teleport },
        { "weapons",  tab_weapons  },
        { "world",    tab_world    },
        { "cheatcodes", tab_cheats },
      };

      for(auto& tab : tabs){
        if(ImGui::BeginTabItem(tab.name)){
          ImGui::BeginChild("##content");
          tab.draw();
          ImGui::EndChild();
          ImGui::EndTabItem();
        }
      }
      ImGui::EndTabBar();
    }

    ImGui::End();
  }
}
