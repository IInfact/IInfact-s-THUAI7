#ifndef FIRE_H_
#define FIRE_H_

#include <fmt/ranges.h>
#include <spdlog/spdlog.h>

#include <vector>

#include "agent/agent.h"
#include "agent/position.h"
#include "agent/supply.h"
#include "agent/map.h"
#include "block.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

float EuclidDistance(thuai7_agent::Position<float> pos1,
    thuai7_agent::Position<float> pos2) {
  float x1 = pos1.x;
  float y1 = pos1.y;
  float x2 = pos2.x;
  float y2 = pos2.y;
  return sqrtf((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}


bool Fire_Condition(thuai7_agent::Map map, thuai7_agent::PlayerInfo self_info,
                    thuai7_agent::PlayerInfo opponent_info) {
  thuai7_agent::Position<float> self_position = self_info.position;
  thuai7_agent::Position<float> opponent_position = opponent_info.position;

  int opponent_weapon_kind = (int)opponent_info.firearm;


  //拳王判定：只要距离低于5格直接开枪
  if (opponent_weapon_kind != 0 && opponent_weapon_kind != 1 &&
      opponent_weapon_kind != 3) {
    if (EuclidDistance(self_position, opponent_position) <= 5) {
      return true;
    }
  }

  //阻挡判定
  if (Is_Blocked(map, self_position, opponent_position)) {
    return false;
  }

  //距离判定
  if (EuclidDistance(self_position, opponent_position) > 120.0) {
    return false;
  }
  return true;
}


#endif