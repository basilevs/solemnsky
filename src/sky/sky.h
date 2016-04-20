/**
 * Physical game state of an Arena.
 */
#pragma once
#include <map>
#include <memory>
#include "physics.h"
#include "plane.h"
#include "arena.h"

namespace sky {

struct SkyInitializer {
  SkyInitializer() = default; // packing
  SkyInitializer(const MapName &mapName);

  template<typename Archive>
  void serialize(Archive &ar) {
    ar(mapName, planes);
  }

  MapName mapName; // the map to load
  std::map<PID, PlaneInitializer> planes; // planes already in the arena
};

struct SkyDelta: public VerifyStructure {
  // TODO: map changes will go here

  SkyDelta() = default; // packing

  template<typename Archive>
  void serialize(Archive &ar) {
    ar(planes);
  }

  bool verifyStructure() const;

  std::map<PID, PlaneDelta> planes;
};

/**
 * A Sky is a subsystem holding the state of a game being played.
 */
class Sky: public Subsystem {
 private:
  friend struct Plane;
  std::map<PID, Plane> planes;
  Plane *planeFromPID(const PID pid);

 protected:
  /**
   * Subsystem implementation.
   */
  void registerPlayer(Player &player) override;
  void unregisterPlayer(Player &player) override;

  void onTick(const float delta) override;
  void onJoin(Player &player) override;
  void onQuit(Player &player) override;
  void onAction(Player &player, const Action action, const bool state) override;
  void onSpawn(Player &player, const PlaneTuning &tuning,
               const sf::Vector2f &pos, const float rot) override;
  // NOTE: unlike the other callbacks, onAction can't be depended on to call
  // everywhere on the network; doAction() doesn't make its way through
  // through deltas. It's only used to effect changes in some subsystems (Sky
  // notably), which in turn enter into the Networked infrastructure.

 public:
  Sky(class Arena &parent, const SkyInitializer &initializer);
  ~Sky();

  const MapName mapName;
  const Map map; // physics uses this at construction
  Physics physics;

  /**
   * API for the user.
   */
  SkyInitializer captureInitializer();
  SkyDelta collectDelta();
  void applyDelta(const SkyDelta &delta);
  Plane &getPlane(const Player &player) const;
};

}
