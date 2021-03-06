/**
 * solemnsky: the open-source multiplayer competitive 2D plane game
 * Copyright (C) 2016  Chris Gadzinski
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "vanilla.hpp"

void VanillaServer::tickGame(const TimeDiff delta, sky::Sky &sky) {
  arena.forPlayers([&](sky::Player &player) {
    auto &participation = sky.getParticipation(player);
    if (participation.isSpawned()) {
      sky::Plane &plane = participation.plane.get();
      for (auto &prop : participation.props) {
        if (prop.second.getLifetime() > 1) {
          prop.second.destroy();
        }
      }

      if (plane.getState().health == 0) {
        participation.suicide();
      }

      if (participation.getControls().getState<sky::Action::Primary>()) {
        if (plane.getState().primaryCooldown) {
          if (plane.requestDiscreteEnergy(0.3)) {
            auto &physical = plane.getState().physical;
            const auto dir = VecMath::fromAngle(physical.rot);
            participation.spawnProp(
                sky::PropInit(
                    physical.pos + (plane.getTuning().hitbox.x / 2.0f) * dir,
                    500.0f * dir));
            plane.resetPrimary();
          }
        }
      }
    }
  });
}

void VanillaServer::onTick(const TimeDiff delta) {
  if (shared.skyHandle.getSky())
    tickGame(delta, *shared.skyHandle.getSky());

}
//Collision
void VanillaServer::onBeginContact(const sky::BodyTag &body1, const sky::BodyTag &body2) {
  //"Death walls"
  if (body1.type == sky::BodyTag::Type::PlaneTag && body2.type == sky::BodyTag::Type::ObstacleTag) {
    body1.plane->damage(body1.plane->getState().health);
  } else if (body2.type == sky::BodyTag::Type::PlaneTag && body1.type == sky::BodyTag::Type::ObstacleTag) {
    body2.plane->damage(body2.plane->getState().health);
  }
}

void VanillaServer::onEndContact(const sky::BodyTag &body1, const sky::BodyTag &body2) {

}

bool VanillaServer::enableContact(const sky::BodyTag &body1, const sky::BodyTag &body2) {
  //Doesn't work for planes because physics is client authoritative
  return false;
}

void VanillaServer::onPacket(ENetPeer *const client,
                             sky::Player &player,
                             const sky::ClientPacket &packet) {
  if (packet.type == sky::ClientPacket::Type::RCon) {
    if (!player.isAdmin()) {
      if (packet.stringData.get() == "auth") {
        sky::PlayerDelta delta{player};
        delta.admin = true;
        shared.registerArenaDelta(sky::ArenaDelta::Delta(player.pid, delta));
        shared.rconResponse(client, "you're authenticated");
      }
    } else {
      // TODO: uniform command parsing & validation

      // Split the command by whitespace
      std::stringstream tmp(packet.stringData.get());
      std::vector<std::string> command{std::istream_iterator<std::string>{tmp},
                                       std::istream_iterator<std::string>{}};

      if (command[0] == "start") {
        if (command.size() > 1) {
          shared.rconResponse(client, "/start -- Starts the game on the next map.");
          return;
        }
        shared.registerArenaDelta(sky::ArenaDelta::Mode(sky::ArenaMode::Game));
        shared.registerGameStart();
        return;
      }

      if (command[0] == "stop") {
        if (command.size() > 1) {
          shared.rconResponse(client, "/stop -- Stops the game and returns to lobby.");
          return;
        }
        shared.registerGameEnd();
        shared.registerArenaDelta(sky::ArenaDelta::Mode(sky::ArenaMode::Lobby));
        return;
      }

      if (command[0] == "restart") {
        if (command.size() > 1) {
          shared.rconResponse(client, "/restart -- Restarts the map.");
          return;
        }
        skyHandle.start();
        return;
      }

      if (command[0] == "map") {
        if (command.size() < 2) {
          shared.rconResponse(client, "/map <name> -- Sets <name> as the next map.");
          return;
        }
        shared.registerArenaDelta(sky::ArenaDelta::EnvChange(command[1]));
        skyHandle.start();
        return;
      }
    }
  }

  if (packet.type == sky::ClientPacket::Type::ReqTeam) {
    if (arena.getMode() == sky::ArenaMode::Lobby) {
      sky::PlayerDelta delta{player};
      delta.team = packet.team.get();
      shared.registerArenaDelta(sky::ArenaDelta::Delta(player.pid, delta));
      assert(player.getTeam() == packet.team.get());
    }
  }

  if (packet.type == sky::ClientPacket::Type::ReqSpawn) {
    if (shared.skyHandle.getSky()) {
      if (player.getTeam() == sky::Team::Spectator) {
        //Spectator trying to join a game, just assign them a team

        //TODO: Make this way more pretty (ie for any number of teams)
        int reds = 0;
        int blues = 0;
        arena.forPlayers([&reds, &blues](const sky::Player &player){
          switch (player.getTeam()) {
            case sky::Team::Red:
              reds ++;
              return;
            case sky::Team::Blue:
              blues ++;
              return;
            case sky::Team::Spectator:
              return;
          }
        });

        sky::Team spawnTeam = sky::Team::Spectator;
        if (reds < blues) {
          spawnTeam = sky::Team::Red;
        } else if (blues > reds) {
          spawnTeam = sky::Team::Blue;
        } else {
          spawnTeam = static_cast<sky::Team>((std::rand() % arena.getTeamCount()) + 1);
        }
        sky::PlayerDelta delta{player};
        delta.team = spawnTeam;
        shared.registerArenaDelta(sky::ArenaDelta::Delta(player.pid, delta));
        assert(player.getTeam() == spawnTeam);
      }

      //Spawn them
      auto sp = shared.skyHandle.getSky()
        ->getMap().pickSpawnPoint(player.getTeam());
      player.spawn({}, sp.pos, sp.angle);
    }
  }
}

VanillaServer::VanillaServer(ServerShared &shared) :
    Server(shared) {}

