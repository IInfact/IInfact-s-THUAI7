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

//��Ϸ�̼���
int tickcount = 0;

//״ָ̬ʾ����
//��mode < 10��˵�����ڱ���ǰ��׼���׶Σ���ʱ���е������ƶ��켣����Ч
//0: ���ּ�ǹ
//1: Ѱ·�����ײ��񻤼�
//10: �����Ѱ·
int mode = 0;

//���ƫ��
float offset_x = 0;
float offset_y = 0;

//��Ϸ�е�һ��Ҫ�ҵĻ��׺͵ڶ���Ҫ�ҵ�Vector���ɿ��ֵĺ���ȷ��
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
  
  //ÿ����Ϸ�̱�Ļ
  spdlog::info("----Game Tick {} (mode {})---- ", tickcount,mode);
  spdlog::info("{}", self_info);


  //��1��Ϸ�̣����ó�����
  if (tickcount == 197) {

    spdlog::info("{}", safe_zone);

    auto my_spawnpoint = ChooseSpawnpoint(agent,initial_weapon,initial_armor);
    agent.ChooseOrigin(my_spawnpoint);


    spdlog::info("Chose spawnpoint at {}", my_spawnpoint);

  }


  //��ģʽ
  switch (mode) { 
      case 0:
           //��ǹ���ӵ�����ǹ
          agent.PickUp((enum thuai7_agent::SupplyKind)4, 4);
          agent.PickUp((enum thuai7_agent::SupplyKind)2, 1);
          agent.SwitchFirearm((enum thuai7_agent::FirearmKind)0);
          if ((int)self_info.firearm == 0) mode = 1;
          break;

      case 1:
          //����Ѱ·�յ㵽���ף��˺�����
          target_position_int = thuai7_agent::Position<int>{
              static_cast<int>(initial_armor.position.x),
              static_cast<int>(initial_armor.position.y)};
          agent.PickUp((enum thuai7_agent::SupplyKind)6, 1);
          agent.PickUp((enum thuai7_agent::SupplyKind)7, 1);
          if ((int)self_info.armor > 0) mode = 2;//���ϻ��׾��е���һ�׶�
          break;
      case 2:
          //����Ѱ·�յ㵽vector���˺����������ǹ
          target_position_int = thuai7_agent::Position<int>{
              static_cast<int>(initial_weapon.position.x),
              static_cast<int>(initial_weapon.position.y)};
          agent.PickUp((enum thuai7_agent::SupplyKind)1, 1);   
          agent.PickUp((enum thuai7_agent::SupplyKind)4, 16);
          agent.SwitchFirearm((enum thuai7_agent::FirearmKind)3);
          if ((int)self_info.firearm == 3) mode = 10;//��ǹ�ɹ����е���һ�׶�
          break;


      case 10:
          //ʼ������Ѱ·�յ�����ӽ���ͼ���ĵ�Ļ�Ѫ���ʻ�߼���
        target_position_int = thuai7_agent::Position<int>{
            static_cast<int>(opponent_info.position.x),
            static_cast<int>(opponent_info.position.y)};
          auto center_aid_position_int = FindCenterAid(agent);
        target_position_int = center_aid_position_int;
          spdlog::info("Going to center aid / PremiumArmor at {}", center_aid_position_int);
      break;

  }




  //�Զ�Ѱ·ģ��
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


  //���ƫ��
  if (mode == 30) {
    std::srand(std::time(nullptr));
    offset_x = (rand() % 100) / 50.0 - 1;
    offset_y = (rand() % 100) / 50.0 - 1;
  }

  //�ƶ�ģ��: Ѱ·״̬��
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



  //�Զ���ʰģ��
  PickUpUnderSupply(agent);
  
  //��ҩģ��
  int my_health = self_info.health;
  if (my_health <= 50) agent.UseMedicine((thuai7_agent::MedicineKind)1);
  if (my_health <= 90) agent.UseMedicine((thuai7_agent::MedicineKind)0);
  if (my_health <= 90) agent.UseMedicine((thuai7_agent::MedicineKind)0);
  if (my_health <= 90) agent.UseMedicine((thuai7_agent::MedicineKind)0);


  //����ģ��
  if (Fire_Condition(map, self_info, opponent_info)) {
    agent.SwitchFirearm((enum thuai7_agent::FirearmKind)3);
    agent.Attack(opponent_info.position);
    agent.SwitchFirearm((enum thuai7_agent::FirearmKind)0);
    agent.Attack(opponent_info.position);
    agent.SwitchFirearm((enum thuai7_agent::FirearmKind)3);
  }




}


