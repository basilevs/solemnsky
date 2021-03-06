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
/**
 * A player's participation in a Sky.
 */
#pragma once
#include <Box2D/Box2D.h>
#include <forward_list>
#include "util/types.hpp"
#include "prop.hpp"
#include "physics.hpp"
#include "planestate.hpp"

namespace sky {

/**
 * The plane element that can be associated with a Participation.
 * This is essentially a piece of Participations's implementation.
 */
class Plane {
  friend class Sky;
  friend class Participation;
 private:
  // Parameters.
  Physics &physics;
  const PlaneControls &controls;

  // State.
  const PlaneTuning tuning;
  PlaneState state;
  b2Body *const body;

  // Subroutines.
  void switchStall();
  void tickFlight(const TimeDiff delta);
  void tickWeapons(const TimeDiff delta);
  void writeToBody();
  void readFromBody();

  // Sky API.
  void prePhysics();
  void postPhysics(const TimeDiff delta);
  void onBeginContact(const BodyTag &body);
  void onEndContact(const BodyTag &body);

 public:
  Plane() = delete;
  Plane(const PID player, Physics &, PlaneControls &&, const PlaneTuning &,
        const PlaneState &) = delete; // `controls` must not be a temp
  Plane(const PID player,
        Physics &physics,
        const PlaneControls &controls,
        const PlaneTuning &tuning,
        const PlaneState &state);
  ~Plane();

  const PID associatedPlayer;

  // User API.
  const PlaneTuning &getTuning() const;
  const PlaneState &getState() const;

  bool requestDiscreteEnergy(const float reqEnergy);
  float requestEnergy(const float reqEnergy);
  void resetPrimary();
  void damage(const float amount);

};

/**
 * Initializer for Participation's Networked impl.
 */
struct ParticipationInit {
  ParticipationInit();
  ParticipationInit(const PlaneControls &controls,
                    const PlaneTuning &tuning,
                    const PlaneState &state);
  ParticipationInit(const PlaneControls &controls);

  template<typename Archive>
  void serialize(Archive &ar) {
    ar(spawn, controls, props);
  }

  optional<std::pair<PlaneTuning, PlaneState>> spawn;
  PlaneControls controls;
  std::map<PID, PropInit> props;

};

/**
 * Delta for Participation's Networked impl.
 */
struct ParticipationDelta: public VerifyStructure {
  ParticipationDelta() = default;

  template<typename Archive>
  void serialize(Archive &ar) {
    ar(spawn, planeAlive, state, serverState, controls);
    ar(propInits, propDeltas);
  }

  bool verifyStructure() const;

  optional<std::pair<PlaneTuning, PlaneState>> spawn;
  bool planeAlive;
  optional<PlaneState> state; // if client doesn't have authority
  optional<PlaneStateServer> serverState; // if client has authority
  optional<PlaneControls> controls; // client authority

  std::map<PID, PropInit> propInits;
  std::map<PID, PropDelta> propDeltas;

  ParticipationDelta respectClientAuthority() const;

};

/**
 * Changes a client can apply to a server's Participation record for that
 * client.
 */
struct ParticipationInput {
  ParticipationInput() = default;

  template<typename Archive>
  void serialize(Archive &ar) {
    ar(planeState, controls);
  }

  optional<PlaneStateClient> planeState;
  optional<PlaneControls> controls;

};

/**
 * A Player's participation in an active game.
 */
class Participation: public Networked<ParticipationInit, ParticipationDelta> {
  friend class Sky;
  friend class SkyHandle;
 private:
  // Parameters.
  Physics &physics;

  // State.
  PlaneControls controls;

  // Delta collection state.
  bool newlyAlive;
  PlaneControls lastControls;

  // Helpers.
  void spawnWithState(const PlaneTuning &tuning,
                      const PlaneState &state);

  // Sky API.
  void doAction(const Action action, bool actionState);
  void prePhysics();
  void postPhysics(const float delta);

  void spawn(const PlaneTuning &tuning,
             const sf::Vector2f &pos,
             const float rot);

 public:
  Participation() = delete;
  Participation(const PID associatedPlayer,
                Physics &physics,
                const ParticipationInit &initializer);

  // State.
  const PID associatedPlayer;
  optional<Plane> plane;
  std::map<PID, Prop> props;

  // Networked impl (for Sky).
  void applyDelta(const ParticipationDelta &delta) override;
  ParticipationInit captureInitializer() const override;
  ParticipationDelta collectDelta();

  // User API.
  const PlaneControls &getControls() const;
  bool isSpawned() const;

  // User API, serverside.
  void spawnProp(const PropInit &init);
  void suicide();

  // ParticipationInput.
  void applyInput(const ParticipationInput &input);
  optional<ParticipationInput> collectInput();

};

}
