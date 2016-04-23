#include <gtest/gtest.h>
#include "sky/arena.h"
#include "util/methods.h"

/**
 * The Subsystem abstraction allows us to modularize state and functionality
 * associated with an Arena.
 */
class SubsystemTest: public testing::Test { };

class LifeSubsystem: public sky::Subsystem {
 private:
  std::map<PID, bool> lives;

 protected:
  void registerPlayer(sky::Player &player) override {
    lives.emplace(player.pid, false);
    player.data.push_back(&lives.at(player.pid));
  }

  void unregisterPlayer(sky::Player &player) override {
    lives.erase(player.pid);
  }

  void onSpawn(sky::Player &player, const sky::PlaneTuning &,
               const sf::Vector2f &, const float) override {
    getPlayerData<bool>(player) = true;
  }

  void onAction(sky::Player &player, const sky::Action action,
                const bool state) override {
    if (action == sky::Action::Suicide) getPlayerData<bool>(player) = false;
  }

 public:
  LifeSubsystem(sky::Arena &arena) : sky::Subsystem(arena) {
    arena.forPlayers([&](sky::Player &player) {
      registerPlayer(player);
    });
  }

  bool getLifeData(const sky::Player &player) {
    return getPlayerData<bool>(player);
  }
};

class CounterSubsystem: public sky::Subsystem {
 private:
  std::map<PID, float> myData;
  LifeSubsystem &lifeSubsystem;

 protected:
  void registerPlayer(sky::Player &player) override {
    myData.emplace(player.pid, 0);
    player.data.push_back(&myData.at(player.pid));
  }

  void unregisterPlayer(sky::Player &player) override {
    myData.erase(player.pid);
  }

  void onTick(const float delta) override {
    arena.forPlayers([&](sky::Player &player) {
      if (lifeSubsystem.getLifeData(player))
        getPlayerData<float>(player) += delta;
    });
  }

 public:
  CounterSubsystem(sky::Arena &arena, LifeSubsystem &lifeSubsystem) :
      sky::Subsystem(arena), lifeSubsystem(lifeSubsystem) {
    arena.forPlayers([&](sky::Player &player) {
      registerPlayer(player);
    });
  }

  float getTimeData(const sky::Player &player) {
    return getPlayerData<float>(player);
  }
};

TEST_F(SubsystemTest, LifeCounter) {
  sky::Arena arena(sky::ArenaInitializer("my arena", "test1"));
  LifeSubsystem lifeSubsystem(arena);
  CounterSubsystem counterSubsystem(arena, lifeSubsystem);

  arena.connectPlayer("player number 1");
  auto &player1 = *arena.getPlayer(0);
  EXPECT_EQ(lifeSubsystem.getLifeData(player1), false);
  EXPECT_EQ(counterSubsystem.getTimeData(player1), 0);
  arena.tick(0.5);
  EXPECT_EQ(counterSubsystem.getTimeData(player1), 0);
  player1.spawn({}, {300, 300}, 0);
  EXPECT_EQ(lifeSubsystem.getLifeData(player1), true);
  arena.tick(0.5);
  EXPECT_EQ(counterSubsystem.getTimeData(player1), 0.5);
  player1.doAction(sky::Action::Suicide, true);
  EXPECT_EQ(lifeSubsystem.getLifeData(player1), false);
}


