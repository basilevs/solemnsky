#include "arena.h"
#include <algorithm>
#include "util/methods.h"
#include "event.h"

namespace sky {

/**
 * Player.
 */

PlayerInitializer::PlayerInitializer(const std::string &nickname) :
    nickname(nickname), admin(false), team(0) { }

Player::Player(const PlayerInitializer &initializer) :
    Networked(initializer),
    pid(initializer.pid),
    nickname(initializer.nickname),
    admin(initializer.admin),
    team(initializer.team) { }

void Player::applyDelta(const PlayerDelta &delta) {
  if (delta.nickname) nickname = *delta.nickname;
  admin = delta.admin;
  if (delta.team) team = *delta.team;
}

PlayerInitializer Player::captureInitializer() const {
  PlayerInitializer initializer;
  initializer.nickname = nickname;
  initializer.admin = admin;
  initializer.team = team;
  return initializer;
}

PlayerDelta Player::zeroDelta() const {
  PlayerDelta delta;
  delta.admin = admin;
  return delta;
}

/**
 * Arena.
 */

ArenaDelta::ArenaDelta(const ArenaDelta::Type type) : type(type) { }

bool ArenaDelta::verifyStructure() const {
  switch (type) {
    case Type::Quit:
      return verifyFields(quit);
    case Type::Join:
      return verifyFields(join);
    case Type::Delta:
      return verifyFields(delta);
    case Type::Motd:
      return verifyFields(motd);
    case Type::Mode:
      return verifyFields(mode);
  }
  return false; // enum out of bounds
}

ArenaDelta ArenaDelta::Quit(const PID pid) {
  ArenaDelta delta(Type::Quit);
  delta.quit = pid;
  return delta;
}

ArenaDelta ArenaDelta::Join(const PID pid, const Player &player) {
  ArenaDelta delta(Type::Join);
  delta.join = std::pair<PID, Player>(pid, player);
  return delta;
}

ArenaDelta ArenaDelta::Delta(const PID pid, const PlayerDelta &delta) {
  ArenaDelta adelta(Type::Delta);
  adelta.delta = std::pair<PID, PlayerDelta>(pid, delta);
  return adelta;
}

ArenaDelta ArenaDelta::Motd(const std::string &motd) {
  ArenaDelta delta(Type::Motd);
  delta.motd = motd;
  return delta;
}

ArenaDelta ArenaDelta::Mode(const ArenaMode arenaMode) {
  ArenaDelta delta(Type::Mode);
  delta.mode = arenaMode;
  return delta;
}

/**
 * Subsystem.
 */

Subsystem::Subsystem(Arena *arena) :
    id(PID(arena->subsystems.size())) {
  arena->subsystems.push_back(this);
  for (auto &player : arena->players) initialize(player.second);
}

void Subsystem::initialize(Player &player) {
  player.data.push_back(nullptr);
}

/**
 * Arena.
 */

ArenaInitializer::ArenaInitializer(const std::string &name) :
    name(name), mode(ArenaMode::Lobby) { }

PID Arena::allocPid() const {
  std::vector<int> usedPids;
  for (const auto &player : players) usedPids.push_back(player.first);
  return (PID) smallestUnused(usedPids);
}

std::string Arena::allocNickname(const std::string &requested) const {
  std::stringstream readStream;
  int nickNumber;
  const size_t rsize = requested.size();
  std::vector<int> usedNumbers;

  for (const auto &player : players) {
    const std::string &name = player.second.nickname;
    appLog("Checking name: " + name);
    if (name.size() < rsize) continue;
    if (name.substr(0, rsize) != requested) continue;

    if (name == requested) {
      usedNumbers.push_back(0);
      continue;
    }

    readStream.str(name.substr(rsize));

    if (readStream.get() != '(') continue;
    readStream >> nickNumber;
    if (!readStream.good()) continue;
    if (readStream.get() != ')') continue;
    readStream.get();
    if (!readStream.eof()) continue;

    usedNumbers.push_back(nickNumber);
  }

  if (usedNumbers.empty()) return requested;

  return requested + "(" + std::to_string(smallestUnused(usedNumbers)) + ")";
}

void Arena::forSubsystems(std::function<void(Subsystem &)> call) {
  for (const Subsystem *system : subsystems) call(*system);
}

void Arena::forPlayers(std::function<void(Player &)> call) {
  for (auto &player : players) call(player.second);
}

Arena::Arena(const ArenaInitializer &initializer) :
    Networked(initializer),
    name(initializer.name),
    motd(initializer.motd),
    mode(initializer.mode) {
  for (auto const &player : initializer.players) {
    players.emplace(player.first, player.second);
  }
}

void Arena::applyDelta(const ArenaDelta &delta) {
  switch (delta.type) {
    case ArenaDelta::Type::Quit: {
      if (Player *player = getPlayer(*delta.quit)) quitPlayer(*player);
      break;
    }

    case ArenaDelta::Type::Join: {
      PlayerInitializer &initializer = *delta.join;
      joinPlayer(initializer);
      break;
    }

    case ArenaDelta::Type::Delta: {
      const PID &pid = delta.delta->first;
      const PlayerDelta &playerDelta = delta.delta->second;

      if (Player *player = getPlayer(pid)) {
        std::string oldNick = player->nickname;
        Team oldTeam = player->team;

        player->applyDelta(playerDelta);

        if (player->nickname != oldNick)
          forSubsystems([&](Subsystem &s) { s.nickChange(*player, oldNick); });
        if (player->team != oldTeam)
          forSubsystems([&](Subsystem &s) { s.teamChange(*player, oldTeam); });
      }
      break;
    }

    case ArenaDelta::Type::Motd: {
      motd = *delta.motd;
      break;
    }

    case ArenaDelta::Type::Mode: {
      mode = *delta.mode;
      break;
    }
  }
}

ArenaInitializer Arena::captureInitializer() const {
  ArenaInitializer initializer;
  for (auto &player : players) {
    initializer.players.emplace(
        player.first, player.second.captureInitializer());
  }
  initializer.name = name;
  initializer.motd = motd;
  initializer.mode = mode;
  return initializer;
}

Player &Arena::joinPlayer(const PlayerInitializer &initializer) {
  if (Player *player = getPlayer(initializer.pid)) quitPlayer(*player);
  players.emplace(initializer.pid, Player(initializer));
  Player &player = players.at(initializer.pid);
  forSubsystems([&player](Subsystem &s) { s.join(player); });
  return player;
}

void Arena::quitPlayer(Player &player) {
  forSubsystems([player](Subsystem &s) { s.quit(player); });
  players.erase(player.pid);
}

Player &Arena::connectPlayer(const std::string &requestedNick) {
  PID pid = allocPid();
  std::string nickname = allocNickname(requestedNick);
  players.emplace(pid, Player(nickname));
  Player &newPlayer = players.at(pid);
  forSubsystems([&newPlayer](Subsystem &s) { s.join(newPlayer); });
  return newPlayer;
}

Player *Arena::getPlayer(const PID pid) {
  auto player = players.find(pid);
  if (player != players.end()) return &player->second;
  return nullptr;
}
}
