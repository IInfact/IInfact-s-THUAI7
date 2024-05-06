#include "agent.h"

#include <fmt/format.h>
#include <hv/EventLoop.h>
#include <hv/WebSocketClient.h>
#include <hv/hloop.h>
#include <spdlog/spdlog.h>

#include <string>
#include <string_view>

#include "agent/map.h"
#include "agent/message.h"
#include "agent/player_info.h"
#include "agent/position.h"
#include "agent/supply.h"
#include "hv/Event.h"
#include "message.h"

namespace thuai7_agent {

Agent::Agent(std::string_view token, hv::EventLoopPtr const& event_loop,
             int loop_interval)
    : event_loop_(event_loop), token_(token) {
  loop_timer_id_ =
      event_loop_->setInterval(loop_interval, [this](hv::TimerID) { Loop(); });

  ws_client_ = std::make_unique<hv::WebSocketClient>(event_loop);

  reconn_setting_t reconn_setting;
  reconn_setting.delay_policy = 0;  // Fixed delay.

  ws_client_->setReconnect(&reconn_setting);

  ws_client_->onmessage = [this](std::string const& msg) {
    OnMessage(Message(msg));
  };
}

Agent::~Agent() { event_loop_->killTimer(loop_timer_id_); }

void Agent::Connect(std::string_view server_address) {
  ws_client_->open(server_address.data());
  // std::this_thread::sleep_for(std::chrono::seconds(5));
}

auto Agent::IsConnected() const -> bool { return ws_client_->isConnected(); }

void Agent::Disconnect() { ws_client_->close(); }

auto Agent::IsGameReady() const -> bool {
  return all_player_info_.has_value() && map_.has_value() &&
         supplies_.has_value() && safe_zone_.has_value() &&
         self_id_.has_value();
}

void Agent::Abandon(SupplyKind target_supply, int count) {
  ws_client_->send(PerformAbandonMessage(count, token_, target_supply).json());
}

void Agent::PickUp(SupplyKind target_supply, int count) {
  ws_client_->send(PerformPickUpMessage(token_, target_supply, count).json());
}

void Agent::SwitchFirearm(FirearmKind target_firearm) {
  ws_client_->send(PerformSwitchArmMessage(token_, target_firearm).json());
}

void Agent::UseMedicine(MedicineKind target_medicine) {
  ws_client_->send(PerformUseMedicineMessage(token_, target_medicine).json());
}

void Agent::UseGrenade(Position<float> const& position) {
  ws_client_->send(PerformUseGrenadeMessage(token_, position).json());
}

void Agent::Move(Position<float> const& position) {
  ws_client_->send(PerformMoveMessage(token_, position).json());
}

void Agent::Stop() { ws_client_->send(PerformStopMessage(token_).json()); }

void Agent::Attack(Position<float> const& position) {
  ws_client_->send(PerformAttackMessage(token_, position).json());
}

void Agent::ChooseOrigin(Position<float> const& position) {
  ws_client_->send(ChooseOriginMessage(token_, position).json());
}

void Agent::Loop() {
  // TODO(mzzy11): Implement loop.
  // spdlog::debug("sending get player info message");
  if (IsConnected()) {
    ws_client_->send(GetPlayerInfoMessage(token_).json());
  }
}

void Agent::OnMessage(Message const& message) {
  auto msg_dict = message.msg;
  try {
    auto msg_type = msg_dict["messageType"].get<std::string>();

    if (msg_type == "ERROR") {
      spdlog::error("error from server: {}",
                    msg_dict["message"].get<std::string>());
    } else if (msg_type == "PLAYERS_INFO") {
      all_player_info_ = std::vector<PlayerInfo>();
      for (auto const& data : msg_dict["players"]) {
        auto player_id = data["playerId"].get<int>();
        auto armor = data["armor"].get<ArmorKind>();
        auto health = data["health"].get<int>();
        auto speed = data["speed"].get<float>();
        auto firearm = data["firearm"]["name"].get<FirearmKind>();
        auto range = data["firearm"]["distance"].get<float>();
        Position<float> position{data["position"]["x"].get<float>(),
                                 data["position"]["y"].get<float>()};
        std::vector<Item> inventory;
        for (auto const& msg_item : data["inventory"]) {
          inventory.emplace_back(Item{msg_item["name"].get<ItemKind>(),
                                      msg_item["num"].get<int>()});
        }
        all_player_info_->emplace_back(PlayerInfo{player_id, armor, health,
                                                  speed, firearm, range,
                                                  position, inventory});
      }
    } else if (msg_type == "MAP") {
      auto length = msg_dict["length"].get<int>();
      std::vector<Position<int>> walls;
      for (auto const& msg_wall : msg_dict["walls"]) {
        walls.emplace_back(
            Position<int>{msg_wall["wallPositions"]["x"].get<int>(),
                          msg_wall["wallPositions"]["y"].get<int>()});
      }
      map_ = Map{length, walls};
    } else if (msg_type == "SUPPLIES") {
      supplies_ = std::vector<Supply>();
      for (auto const& msg_supply : msg_dict["supplies"]) {
        auto kind = msg_supply["name"].get<SupplyKind>();
        Position<float> position{msg_supply["position"]["x"].get<float>(),
                                 msg_supply["position"]["y"].get<float>()};
        auto count = msg_supply["numb"].get<int>();

        supplies_->emplace_back(Supply{kind, count, position});
      }
    } else if (msg_type == "SAFE_ZONE") {
      Position<float> center{msg_dict["center"]["x"].get<float>(),
                             msg_dict["center"]["y"].get<float>()};
      auto radius = msg_dict["radius"].get<float>();
      safe_zone_ = SafeZone{center, radius};
    } else if (msg_type == "PLAYER_ID") {
      self_id_ = msg_dict["playerId"].get<int>();
    } else {
      spdlog::warn("unknown message type: {}", msg_type);
    }
  } catch (std::exception const& e) {
    spdlog::error("error occurred in message handling: {}", e.what());
  }
}

auto format_as(Agent const& object) -> std::string {
  return fmt::format("Agent{{token: {}}}", object.token());
}

}  // namespace thuai7_agent
