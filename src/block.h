#ifndef BLOCK_H_
#define BLOCK_H_

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

bool Is_Blocked(thuai7_agent::Map map, thuai7_agent::Position<float> pos1,
    thuai7_agent::Position<float> pos2) {

  thuai7_agent::Position<int> flag_position; //向下取整的位置，用于判断是否遇到障碍
    
  float x1 = pos1.x;
  float x2 = pos2.x;
  float y1 = pos1.y;
  float y2 = pos2.y;

  if (x1 > x2) {
    std::swap(x1, x2);
    std::swap(y1, y2);
  }

  if (x2 - x1 < 10e-3) {
    if (y2 > y1) {
      for (int i = static_cast<int> (y1); i <= static_cast<int> (y2); i++) {
        flag_position.x = static_cast<int> (x1);
        flag_position.y = i;
        if (std::find(map.obstacles.begin(), map.obstacles.end(),
                      flag_position) != map.obstacles.end()) {
          return true;
        }
      }
    }
    if (y2 < y1) {
      for (int i = static_cast<int> (y1); i <= static_cast<int> (y2); i--) {
        flag_position.x = static_cast<int> (x1);
        flag_position.y = i;
        if (std::find(map.obstacles.begin(), map.obstacles.end(),
                      flag_position) != map.obstacles.end()) {
          return true;
        }
      }
    }
  }

  float k = (y2 - y1) / (x2 - x1); //斜率
  float l = (x2 - x1) / 256.0; //步长
  for (int i = 0; i <= 256; i++) {
    flag_position.x = static_cast<int>(x1 + i * l);
    flag_position.y = static_cast<int>(y1 + i * k * l);
    if (std::find(map.obstacles.begin(), map.obstacles.end(), flag_position) !=
        map.obstacles.end()) {
      return true;
    }
  }


  return false;
}




#endif