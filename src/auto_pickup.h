#ifndef AUTO_PICKUP_H_
#define AUTO_PICKUP_H_

#include <fmt/ranges.h>
#include <spdlog/spdlog.h>

#include <vector>

#include "agent/agent.h"
#include "agent/position.h"
#include "agent/supply.h"
#include "agent/map.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

int InventoryUsed(auto const& inventory) {
  int used = 0;
  for (thuai7_agent::Item each_item : inventory) {
    switch ((int)each_item.kind) {
      case 0:
        used += 2 * each_item.count;
        break;
      case 1:
        used += 1 * each_item.count;
        break;
      case 2:
        used += 10 * each_item.count;
        break;
      case 3:
        used += 15 * each_item.count;
        break;
    }
  }
  return used;
}

void PickUpUnderSupply(thuai7_agent::Agent& agent) {

  auto const& player_info_list = agent.all_player_info().value().get();

  auto const& supplies = agent.supplies().value().get();

  auto const self_id = agent.self_id().value();

  auto const& self_info = player_info_list.at(self_id);

  auto const& inventory = self_info.inventory;

  thuai7_agent::Supply supply_to_pick_up = supplies[0];
  int amount = 0;

    auto self_position_int =
      thuai7_agent::Position<int>{static_cast<int>(self_info.position.x),
                                  static_cast<int>(self_info.position.y)};

  for (thuai7_agent::Supply each_supply : supplies) {
    auto supply_position_int =
        thuai7_agent::Position<int>{static_cast<int>(each_supply.position.x),
                                    static_cast<int>(each_supply.position.y)};
    if (supply_position_int.x == self_position_int.x &&
        supply_position_int.y == self_position_int.y) {
      spdlog::info("Find under my foot: {}", each_supply);
      if ((int)each_supply.kind <= 3) continue;
      supply_to_pick_up = each_supply;
      amount = each_supply.count;
      break;
    }
  }

  if (amount == 0) return;
  switch ((int)supply_to_pick_up.kind) { 
      case 4:
      if (InventoryUsed(inventory) <= 60) {
          agent.PickUp(supply_to_pick_up.kind, amount);
      }
      break;
      case 5:
        if (InventoryUsed(inventory) <= 80) {
          agent.PickUp(supply_to_pick_up.kind, amount);
        }
      break;
      case 8:
        if (InventoryUsed(inventory) <= 90) {
          agent.PickUp(supply_to_pick_up.kind, amount);
        }
      break;
      case 7:
        agent.PickUp((enum thuai7_agent::SupplyKind)7, 1);
      break;
  }
}






#endif