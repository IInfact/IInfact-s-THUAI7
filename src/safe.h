#ifndef SAFE_H_
#define SAFE_H_

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



bool OutOfSafeZone(thuai7_agent::SafeZone safe_zone, thuai7_agent::Position<float> pos) {

  thuai7_agent::Position<float> safe_center = safe_zone.center;

  float radius2 = safe_zone.radius * safe_zone.radius;

  float distance = (pos.x - safe_center.x) * (pos.x - safe_center.x) +
                   (pos.y - safe_center.y) * (pos.y - safe_center.y);

  if (distance < radius2 - 2)
    return false;
  else
    return true;
}


#endif