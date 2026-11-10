#pragma once

namespace sa
{
struct VehicleModel { int id; const char* name; };
struct VehicleGroup { const char* name; const VehicleModel* models; int count; };
#define SA_GROUP(name, arr) {name, arr, (int)(sizeof(arr) / sizeof(arr[0]))}

inline const VehicleModel kSports[] = {
    {411, "Infernus"}, {415, "Cheetah"}, {429, "Banshee"}, {451, "Turismo"}, {477, "ZR-350"}, {480, "Comet"},
    {494, "Hotring"}, {502, "Hotring Racer A"}, {503, "Hotring Racer B"}, {506, "Super GT"}, {541, "Bullet"},
    {559, "Jester"}, {560, "Sultan"}, {562, "Elegy"}, {565, "Flash"}, {587, "Euros"}, {589, "Club"},
    {602, "Alpha"}, {603, "Phoenix"}, {402, "Buffalo"}, {558, "Uranus"}, {561, "Stratum"},
};
inline const VehicleModel kSaloons[] = {
    {401, "Bravura"}, {405, "Sentinel"}, {410, "Manana"}, {419, "Esperanto"}, {421, "Washington"},
    {426, "Premier"}, {436, "Previon"}, {445, "Admiral"}, {466, "Glendale"}, {467, "Oceanic"}, {474, "Hermes"},
    {491, "Virgo"}, {492, "Greenwood"}, {507, "Elegant"}, {516, "Nebula"}, {517, "Majestic"}, {526, "Fortune"},
    {527, "Cadrona"}, {529, "Willard"}, {540, "Vincent"}, {546, "Intruder"}, {547, "Primo"}, {549, "Tampa"},
    {550, "Sunrise"}, {551, "Merit"}, {555, "Windsor"}, {566, "Tahoma"}, {580, "Stafford"}, {585, "Emperor"},
};
inline const VehicleModel kMuscle[] = {
    {412, "Voodoo"}, {439, "Stallion"}, {475, "Sabre"}, {496, "Blista Compact"}, {518, "Buccaneer"},
    {533, "Feltzer"}, {534, "Remington"}, {535, "Slamvan"}, {536, "Blade"}, {542, "Clover"}, {545, "Hustler"},
    {567, "Savanna"}, {575, "Broadway"}, {576, "Tornado"},
};
inline const VehicleModel kOffroad[] = {
    {400, "Landstalker"}, {404, "Perennial"}, {418, "Moonbeam"}, {422, "Bobcat"}, {424, "BF Injection"},
    {444, "Monster"}, {458, "Solair"}, {470, "Patriot"}, {478, "Walton"}, {479, "Regina"}, {489, "Rancher"},
    {495, "Sandking"}, {500, "Mesa"}, {505, "Rancher (lure)"}, {543, "Sadler"}, {554, "Yosemite"},
    {556, "Monster A"}, {557, "Monster B"}, {568, "Bandito"}, {573, "Dune"}, {579, "Huntley"}, {600, "Picador"},
};
inline const VehicleModel kService[] = {
    {407, "Fire Truck"}, {416, "Ambulance"}, {420, "Taxi"}, {427, "Enforcer"}, {428, "Securicar"},
    {431, "Bus"}, {432, "Rhino"}, {433, "Barracks"}, {437, "Coach"}, {438, "Cabbie"}, {490, "FBI Rancher"},
    {523, "HPV1000 (police bike)"}, {528, "FBI Truck"}, {544, "Fire Truck (ladder)"}, {596, "Police (LS)"},
    {597, "Police (SF)"}, {598, "Police (LV)"}, {599, "Police Ranger"}, {601, "S.W.A.T."},
};
inline const VehicleModel kIndustrial[] = {
    {403, "Linerunner"}, {406, "Dumper"}, {408, "Trashmaster"}, {413, "Pony"}, {414, "Mule"},
    {423, "Mr Whoopee"}, {440, "Rumpo"}, {443, "Packer"}, {455, "Flatbed"}, {456, "Yankee"},
    {459, "Berkley's RC Van"}, {482, "Burrito"}, {483, "Camper"}, {485, "Baggage"}, {486, "Dozer"},
    {498, "Boxville"}, {499, "Benson"}, {508, "Journey"}, {514, "Tanker"}, {515, "Roadtrain"},
    {524, "Cement Truck"}, {525, "Tow Truck"}, {530, "Forklift"}, {531, "Tractor"}, {532, "Combine"},
    {552, "Utility Van"}, {571, "Kart"}, {572, "Mower"}, {574, "Sweeper"}, {578, "DFT-30"}, {582, "Newsvan"},
    {583, "Tug"}, {588, "Hotdog"}, {609, "Boxville (black)"}, {409, "Stretch"}, {442, "Romero"}, {457, "Caddy"},
    {504, "Bloodring Banger"}, {434, "Hotknife"},
};
inline const VehicleModel kBikes[] = {
    {448, "Pizzaboy"}, {461, "PCJ-600"}, {462, "Faggio"}, {463, "Freeway"}, {468, "Sanchez"}, {471, "Quad"},
    {481, "BMX"}, {509, "Bike"}, {510, "Mountain Bike"}, {521, "FCR-900"}, {522, "NRG-500"}, {581, "BF-400"},
    {586, "Wayfarer"},
};
inline const VehicleModel kAir[] = {
    {460, "Skimmer"}, {476, "Rustler"}, {511, "Beagle"}, {512, "Cropduster"}, {513, "Stuntplane"},
    {519, "Shamal"}, {520, "Hydra"}, {553, "Nevada"}, {577, "AT-400"}, {592, "Andromada"}, {593, "Dodo"},
    {417, "Leviathan"}, {425, "Hunter"}, {447, "Seasparrow"}, {469, "Sparrow"}, {487, "Maverick"},
    {488, "News Chopper"}, {497, "Police Maverick"}, {548, "Cargobob"}, {563, "Raindance"},
};
inline const VehicleModel kBoats[] = {
    {430, "Predator"}, {446, "Squalo"}, {452, "Speeder"}, {453, "Reefer"}, {454, "Tropic"}, {472, "Coastguard"},
    {473, "Dinghy"}, {484, "Marquis"}, {493, "Jetmax"}, {539, "Vortex"}, {595, "Launch"},
};
inline const VehicleModel kRc[] = {
    {441, "RC Bandit"}, {464, "RC Baron"}, {465, "RC Raider"}, {501, "RC Goblin"}, {564, "RC Tiger"}, {594, "RC Cam"},
};
inline const VehicleGroup kVehicleGroups[] = {
    SA_GROUP("Sports cars", kSports),       SA_GROUP("Saloons", kSaloons),
    SA_GROUP("Muscle & lowriders", kMuscle), SA_GROUP("Off-road & SUVs", kOffroad),
    SA_GROUP("Emergency & military", kService), SA_GROUP("Industrial & fun", kIndustrial),
    SA_GROUP("Bikes", kBikes),               SA_GROUP("Planes & helicopters", kAir),
    SA_GROUP("Boats", kBoats),               SA_GROUP("RC vehicles", kRc),
};

inline const char* const kWeather[] = {
    "Extra sunny (LS)", "Sunny (LS)", "Extra sunny smog (LS)", "Sunny smog (LS)", "Cloudy (LS)",
    "Sunny (SF)", "Extra sunny (SF)", "Cloudy (SF)", "Rainy (SF)", "Foggy (SF)",
    "Sunny (LV)", "Extra sunny (LV)", "Cloudy (LV)", "Extra sunny (country)", "Sunny (country)",
    "Cloudy (country)", "Thunderstorm (country)", "Extra sunny (desert)", "Sunny (desert)", "Sandstorm (desert)",
    "Underwater", "Extra colours 1", "Extra colours 2",
};

struct CheatInfo { int index; const char* name; };
inline const CheatInfo kPlayerCheats[] = {
    {62, "Infinite health"}, {63, "Infinite oxygen"}, {69, "Never hungry"}, {61, "Mega jump"},
    {68, "Mega punch"}, {50, "Huge bunny hop"}, {72, "Adrenaline mode"}, {66, "Never wanted (game cheat)"},
    {67, "Six-star wanted level"}, {46, "Women love you"}, {29, "Suicide"},
};
inline const CheatInfo kStatCheats[] = {
    {37, "Max muscle"}, {36, "Max fat"}, {39, "Skinny"}, {82, "Max stamina"}, {80, "Max respect"},
    {81, "Max sex appeal"}, {83, "Hitman level, all weapons"}, {84, "Max vehicle skills"}, {38, "Max gambling skill"},
};
inline const CheatInfo kWeaponCheats[] = {
    {73, "Infinite ammo, no reload (game cheat)"}, {64, "Get parachute"}, {65, "Get jetpack"},
    {77, "Recruit anyone (pistols)"}, {78, "Recruit anyone (AK-47s)"}, {79, "Recruit anyone (rockets)"},
};
inline const CheatInfo kVehicleCheats[] = {
    {28, "Perfect handling"}, {54, "All cars have nitro"}, {49, "Flying cars"}, {35, "Boats fly"},
    {34, "Cars drive on water"}, {27, "Invisible cars"}, {55, "Cars float away when hit"},
    {53, "Cars explode when touched"}, {30, "All traffic lights green"}, {26, "Blow up all cars"},
};
inline const CheatInfo kTrafficCheats[] = {
    {31, "Aggressive drivers"}, {32, "Pink traffic"}, {33, "Black traffic"}, {47, "Cheap traffic"},
    {48, "Fast traffic"}, {75, "Reduced traffic"}, {76, "Country traffic"},
};
inline const CheatInfo kPedCheats[] = {
    {70, "Riot mode"}, {14, "Peds riot"}, {15, "Everyone attacks you"}, {16, "Peds have weapons"},
    {41, "Peds attack with rockets"}, {43, "Gang members everywhere"}, {44, "Gangs control the streets"},
    {45, "Ninja theme"}, {40, "Elvis everywhere"}, {42, "Beach party"}, {71, "Funhouse theme"},
};
inline const CheatInfo kWorldCheats[] = {
    {56, "Always midnight"}, {57, "Orange sky (stop clock)"}, {11, "Faster clock"},
    {12, "Faster gameplay"}, {13, "Slower gameplay"},
};

struct Place { const char* name; float x, y, z; };
inline const Place kPlaces[] = {
    {"Grove Street", 2495.0f, -1687.0f, 13.5f},
    {"Los Santos Airport", 1642.0f, -2238.0f, 13.5f},
    {"Unity Station", 1743.0f, -1862.0f, 13.6f},
    {"Santa Maria Beach", 368.0f, -1958.0f, 7.7f},
    {"Madd Dogg's mansion", 1257.0f, -785.0f, 92.0f},
    {"Palomino Creek", 2337.0f, 37.0f, 26.5f},
    {"Angel Pine", -2165.0f, -2385.0f, 30.6f},
    {"Mount Chiliad (summit)", -2321.0f, -1638.0f, 483.7f},
    {"San Fierro Airport", -1355.0f, -230.0f, 14.2f},
    {"Doherty Garage (SF)", -2026.0f, 156.0f, 29.0f},
    {"Fort Carson", -150.0f, 1095.0f, 19.7f},
    {"Area 69 (gate)", 213.0f, 1867.0f, 17.6f},
    {"Verdant Meadows airstrip", 410.0f, 2533.0f, 16.5f},
    {"The Strip (LV)", 2036.0f, 1008.0f, 10.8f},
};

struct WeaponModel { int type, model, ammo; const char* name; };
struct WeaponGroup { const char* name; const WeaponModel* weapons; int count; };
inline const WeaponModel kMelee[] = {
    {1, 331, 1, "Brass knuckles"}, {2, 333, 1, "Golf club"}, {3, 334, 1, "Nightstick"}, {4, 335, 1, "Knife"},
    {5, 336, 1, "Baseball bat"}, {6, 337, 1, "Shovel"}, {7, 338, 1, "Pool cue"}, {8, 339, 1, "Katana"},
    {9, 341, 1, "Chainsaw"}, {15, 326, 1, "Cane"}, {14, 325, 1, "Flowers"}, {10, 321, 1, "Purple dildo"},
    {11, 322, 1, "Dildo"}, {12, 323, 1, "Vibrator"}, {13, 324, 1, "Silver vibrator"},
};
inline const WeaponModel kPistols[] = {
    {22, 346, 500, "9mm pistol"}, {23, 347, 500, "Silenced 9mm"}, {24, 348, 500, "Desert Eagle"},
};
inline const WeaponModel kShotguns[] = {
    {25, 349, 300, "Shotgun"}, {26, 350, 300, "Sawn-off shotgun"}, {27, 351, 300, "Combat shotgun"},
};
inline const WeaponModel kSmgs[] = {{28, 352, 1000, "Micro Uzi"}, {32, 372, 1000, "Tec-9"}, {29, 353, 1000, "MP5"}};
inline const WeaponModel kRifles[] = {
    {30, 355, 1000, "AK-47"}, {31, 356, 1000, "M4"}, {33, 357, 200, "Country rifle"}, {34, 358, 200, "Sniper rifle"},
};
inline const WeaponModel kHeavy[] = {
    {35, 359, 50, "Rocket launcher"}, {36, 360, 50, "Heat-seeking RPG"}, {37, 361, 1000, "Flamethrower"},
    {38, 362, 2000, "Minigun"},
};
inline const WeaponModel kThrown[] = {
    {16, 342, 25, "Grenades"}, {17, 343, 25, "Tear gas"}, {18, 344, 25, "Molotov cocktails"},
    {39, 363, 25, "Satchel charges"},
};
inline const WeaponModel kGear[] = {
    {46, 371, 1, "Parachute"}, {41, 365, 1000, "Spray can"}, {42, 366, 1000, "Fire extinguisher"},
    {43, 367, 100, "Camera"}, {44, 368, 1, "Night vision goggles"}, {45, 369, 1, "Thermal goggles"},
};
inline const WeaponGroup kWeaponGroups[] = {
    SA_GROUP("Melee", kMelee),   SA_GROUP("Pistols", kPistols),          SA_GROUP("Shotguns", kShotguns),
    SA_GROUP("SMGs", kSmgs),     SA_GROUP("Rifles", kRifles),            SA_GROUP("Heavy weapons", kHeavy),
    SA_GROUP("Thrown", kThrown), SA_GROUP("Gear & gadgets", kGear),
};
}
