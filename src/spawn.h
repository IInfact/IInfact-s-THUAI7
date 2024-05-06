#ifndef SPAWN_H_
#define SPAWN_H_

#include <fmt/ranges.h>
#include <spdlog/spdlog.h>

#include <vector>

#include "agent/agent.h"
#include "agent/position.h"
#include "agent/supply.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>




int ManhattanDistance(float x1, float y1, float x2, float y2) {
  return std::abs((int)x1 - (int)x2) + std::abs((int)y1 - (int)y2);
}

auto ChooseSpawnpoint(thuai7_agent::Agent& agent) {
  auto const& player_info_list = agent.all_player_info().value().get();
  auto const& supplies = agent.supplies().value().get();
  auto const& game_map = agent.map();
  auto const self_id = agent.self_id().value();
  auto const& self_info = player_info_list.at(self_id);
  int nearest_armor_distance = 512;
  thuai7_agent::Supply nearest_armor = supplies[0];
  int final_distance = 512;
  thuai7_agent::Supply final_armor = supplies[0];
  thuai7_agent::Supply final_weapon = supplies[0];

  auto self_position = self_info.position;

  for (thuai7_agent::Supply each_weapon : supplies) {
    if ((int)each_weapon.kind == 2) {
      spdlog::info("Find AWM at {}", each_weapon.position);
      for (thuai7_agent::Supply each_armor : supplies) {
        if ((int)each_armor.kind == 6 || (int)each_armor.kind == 7) {
          if (ManhattanDistance(each_armor.position.x, each_armor.position.y,
                                each_weapon.position.x,
                                each_weapon.position.y) <
              nearest_armor_distance) {
            nearest_armor_distance =
                ManhattanDistance(each_armor.position.x, each_armor.position.y,
                                  each_weapon.position.x, each_weapon.position.y);
            nearest_armor = each_armor;

          }
        }
      }
      if (nearest_armor_distance < final_distance) {
        final_distance = nearest_armor_distance;
        final_armor = nearest_armor;
        final_weapon = each_weapon;
      }
    }
  }
  spdlog::info("Choose to spawn at AWM at {}", final_weapon.position);
  spdlog::info("Nearest armor is {}, at {}, distance is {}", final_armor.kind,
               final_armor.position, final_distance);
  return final_weapon.position;
}
    

#endif