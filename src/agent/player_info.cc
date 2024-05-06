#include "player_info.h"

#include <fmt/ranges.h>

#include <magic_enum.hpp>
#include <string>

namespace thuai7_agent {

auto format_as(ArmorKind object) -> std::string {
  return std::string(magic_enum::enum_name(object));
}

auto format_as(FirearmKind object) -> std::string {
  return std::string(magic_enum::enum_name(object));
}

auto format_as(ItemKind object) -> std::string {
  return std::string(magic_enum::enum_name(object));
}

auto format_as(Item const& object) -> std::string {
  return fmt::format("Item{{kind: {}, count: {}}}", object.kind, object.count);
}

auto format_as(PlayerInfo const& object) -> std::string {
  return fmt::format(
      "PlayerInfo{{id: {}, armor: {}, health: {}, speed: {}, firearm: {}, "
      "range: {}, position: {}, inventory: {}}}",
      object.id, object.armor, object.health, object.speed, object.firearm,
      object.range, object.position, object.inventory);
}

}  // namespace thuai7_agent
