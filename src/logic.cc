#include <fmt/ranges.h>
#include <spdlog/spdlog.h>

#include <vector>
#include <random>
#include <ctime>

#include "agent/agent.h"
#include "agent/position.h"
#include "path_finding.h"
#include "agent/supply.h"

#include "block.h"
#include "spawn.h"
#include "fire.h"
#include "auto_pickup.h"
#include "choose_position.h"

constexpr auto kFloatPositionShift = 0.5;

//游戏刻计数
int tickcount = 0;

//状态指示变量
//当mode < 10，说明正在比赛前期准备阶段，此时所有的修正移动轨迹不生效
//0: 开局捡枪
//1: 寻路至护甲并捡护甲
//10: 向敌人寻路
int mode = 0;

//添加偏移
float offset_x = 0;
float offset_y = 0;

//游戏中第一个要找的护甲和第二个要找的Vector，由开局的函数确定
thuai7_agent::Supply initial_armor;
thuai7_agent::Supply initial_weapon;

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
static struct {
  std::vector<thuai7_agent::Position<int>> path;
} state;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

void Setup(thuai7_agent::Agent& agent) {
  // Your code here.
}

void Loop(thuai7_agent::Agent& agent) {
  // Your code here.
  // Here is an example of how to use the agent.
  // Always move to the opponent's position, keep one cell away from the
  // opponent, and attack the opponent.


  tickcount++;


  auto const& player_info_list = agent.all_player_info().value().get();

  auto const& supplies = agent.supplies().value().get();

  auto const self_id = agent.self_id().value();

  auto const& self_info = player_info_list.at(self_id);

  auto const& opponent_info = player_info_list.at(1 - self_info.id);

  auto const& map = agent.map().value().get();

  auto const& safe_zone = agent.safe_zone().value().get();

  auto self_position_int =
      thuai7_agent::Position<int>{static_cast<int>(self_info.position.x),
                                  static_cast<int>(self_info.position.y)};
  auto target_position_int =
      thuai7_agent::Position<int>{static_cast<int>(self_info.position.x),
                                  static_cast<int>(self_info.position.y)};

  thuai7_agent::Supply target_weapon = supplies[0]; 
  thuai7_agent::Supply target_armor = supplies[0]; 
  thuai7_agent::Supply target_medicine = supplies[0]; 

  enum thuai7_agent::SupplyKind supply_to_pickup =
      (enum thuai7_agent::SupplyKind)0; 
  
  //每个游戏刻报幕
  spdlog::info("----Game Tick {} (mode {})---- ", tickcount,mode);
  spdlog::info("{}", self_info);


  //第1游戏刻，设置出生点
  if (tickcount == 197) {

    spdlog::info("{}", safe_zone);

    auto my_spawnpoint = ChooseSpawnpoint(agent,initial_weapon,initial_armor);
    agent.ChooseOrigin(my_spawnpoint);


    spdlog::info("Chose spawnpoint at {}", my_spawnpoint);

  }


  //分模式
  switch (mode) { 
      case 0:
           //捡枪和子弹并换枪
          agent.PickUp((enum thuai7_agent::SupplyKind)4, 4);
          agent.PickUp((enum thuai7_agent::SupplyKind)2, 1);
          agent.SwitchFirearm((enum thuai7_agent::FirearmKind)0);
          if ((int)self_info.firearm == 0) mode = 1;
          break;

      case 1:
          //设置寻路终点到护甲，此后捡盔甲
          target_position_int = thuai7_agent::Position<int>{
              static_cast<int>(initial_armor.position.x),
              static_cast<int>(initial_armor.position.y)};
          agent.PickUp((enum thuai7_agent::SupplyKind)6, 1);
          agent.PickUp((enum thuai7_agent::SupplyKind)7, 1);
          if ((int)self_info.armor > 0) mode = 2;//穿上护甲就切到下一阶段
          break;
      case 2:
          //设置寻路终点到vector，此后捡起来并换枪
          target_position_int = thuai7_agent::Position<int>{
              static_cast<int>(initial_weapon.position.x),
              static_cast<int>(initial_weapon.position.y)};
          agent.PickUp((enum thuai7_agent::SupplyKind)1, 1);   
          agent.PickUp((enum thuai7_agent::SupplyKind)4, 16);
          agent.SwitchFirearm((enum thuai7_agent::FirearmKind)3);
          if ((int)self_info.firearm == 3) mode = 10;//换枪成功就切到下一阶段
          break;


      case 10:
          //始终设置寻路终点是最接近地图中心点的回血物资或高级甲
        target_position_int = thuai7_agent::Position<int>{
            static_cast<int>(opponent_info.position.x),
            static_cast<int>(opponent_info.position.y)};
          auto center_aid_position_int = FindCenterAid(agent);
        target_position_int = center_aid_position_int;
          spdlog::info("Going to center aid / PremiumArmor at {}", center_aid_position_int);
      break;

  }




  //自动寻路模块
  if (std::find(state.path.begin(), state.path.end(), self_position_int) ==
          state.path.end() ||
      std::find(state.path.begin(), state.path.end(), target_position_int) ==
          state.path.end()) {
    state.path = FindPathBeFS(map, self_position_int, target_position_int);

    if (state.path.empty()) {
      spdlog::info("no path from {} to {}", self_position_int,
                   target_position_int);
      return;
    }

    spdlog::info("found path from {} to {}", self_position_int,
                 target_position_int);
  }

  while (state.path.front() != self_position_int) {
    state.path.erase(state.path.begin());
  }


  //添加偏移
  if (mode == 30) {
    std::srand(std::time(nullptr));
    offset_x = (rand() % 100) / 50.0 - 1;
    offset_y = (rand() % 100) / 50.0 - 1;
  }

  //移动模块: 寻路状态下
  if (state.path.size() > 1) {
    auto next_position_int = state.path.at(1);
    auto next_position = thuai7_agent::Position<float>{
        static_cast<float>(next_position_int.x + kFloatPositionShift +
                           offset_x),
        static_cast<float>(next_position_int.y + kFloatPositionShift +
                           offset_y)};

    agent.Move(next_position);
    //spdlog::info("moving from {} to {}", self_info.position, next_position);
    return;
  }



  //自动捡拾模块
  PickUpUnderSupply(agent);
  
  //吃药模块
  int my_health = self_info.health;
  if (my_health <= 50) agent.UseMedicine((thuai7_agent::MedicineKind)1);
  if (my_health <= 90) agent.UseMedicine((thuai7_agent::MedicineKind)0);
  if (my_health <= 90) agent.UseMedicine((thuai7_agent::MedicineKind)0);
  if (my_health <= 90) agent.UseMedicine((thuai7_agent::MedicineKind)0);


  //开火模块
  if (Fire_Condition(map, self_info, opponent_info)) {
    agent.SwitchFirearm((enum thuai7_agent::FirearmKind)3);
    agent.Attack(opponent_info.position);
    agent.SwitchFirearm((enum thuai7_agent::FirearmKind)0);
    agent.Attack(opponent_info.position);
    agent.SwitchFirearm((enum thuai7_agent::FirearmKind)3);
  }




}


