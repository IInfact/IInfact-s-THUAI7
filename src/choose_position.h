#ifndef CHOOSE_POSITION_H_
#define CHOOSE_POSITION_H_

#include <fmt/ranges.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "agent/agent.h"
#include "agent/map.h"
#include "agent/position.h"
#include "agent/supply.h"

thuai7_agent::Position<int> FindCenterAid(thuai7_agent::Agent& agent) {
  auto supplies = agent.supplies().value().get();
  auto nearest_aid = supplies[0];
  int nearest_distance = 512;
  int distance = 0;

  int another_distance = 0;
  int another_nearest_distance = 512;

  auto const& player_info_list = agent.all_player_info().value().get();

  auto const self_id = agent.self_id().value();

  auto const& self_info = player_info_list.at(self_id);

  auto safe_zone = agent.safe_zone().value().get();

  auto center_position_int =
      thuai7_agent::Position<int>{static_cast<int>(safe_zone.center.x),
                                  static_cast<int>(safe_zone.center.y)};

  for (thuai7_agent::Supply each_supply : supplies) {
    if ((int)each_supply.kind == 5 || (int)each_supply.kind == 8) {
      auto supply_position_int =
          thuai7_agent::Position<int>{static_cast<int>(each_supply.position.x),
                                      static_cast<int>(each_supply.position.y)};
      distance = std::abs(supply_position_int.x - center_position_int.x) +
                 std::abs(supply_position_int.y - center_position_int.y);
      if (distance < nearest_distance) {
        nearest_distance = distance;
        nearest_aid = each_supply;
      }
    }
  }

  if ((int)self_info.armor < 2) {
    for (thuai7_agent::Supply each_armor : supplies) {

      if ((int)each_armor.kind == 7) {
        auto armor_position_int = thuai7_agent::Position<int>{
            static_cast<int>(each_armor.position.x),
            static_cast<int>(each_armor.position.y)};
        another_distance =
            std::abs(armor_position_int.x - center_position_int.x) +
            std::abs(armor_position_int.y - center_position_int.y);
        if (another_distance < another_nearest_distance) {
          another_nearest_distance = another_distance;
          if (another_nearest_distance <= nearest_distance) {
            nearest_aid = each_armor;
          }
        }
      }

    }

  }

  auto nearest_position_int =
      thuai7_agent::Position<int>{static_cast<int>(nearest_aid.position.x),
                                  static_cast<int>(nearest_aid.position.y)};

  if (nearest_distance > 25) {
    return thuai7_agent::Position<int>{center_position_int.x,
                                       center_position_int.y};
  }

  return nearest_position_int;
}

#endif
