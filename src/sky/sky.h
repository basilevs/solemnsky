/**
 * Core game state. The sky, really. Subordinate to Arena.
 */
#pragma once
#include <map>
#include "subsystem.h"
#include <memory>
#include "physics.h"
#include "plane.h"
#include "map.h"

namespace sky {

struct SkyInitializer {
  SkyInitializer();

  template<typename Archive>
  void serialize(Archive &ar) {
    ar(mapName, planes);
  }

  MapName mapName; // the map to load
  std::map<PID, PlaneInitializer> planes; // planes already in the arena
};

struct SkyDelta: public VerifyStructure {
  SkyDelta();

  template<typename Archive>
  void serialize(Archive &ar) {
    ar(state);
  }

  bool verifyStructure() const;

  std::map<PID, PlaneDelta> state;
};

/*
 * A Sky is the state of a game being played. It is held by an Arena when a
 * game is under way.
 */
class Sky: public Subsystem {
 private:
  std::map<PID, Plane> planes;
  Plane *getPlane(const PID pid);

 protected:
  friend class Arena;

  /**
   * Subsystem implementation.
   */
  virtual void onTick(const float delta) override;
  virtual void onJoin(struct Player &player) override;
  virtual void onQuit(struct Player &player) override;

 public:
  Sky(class Arena *parent, const MapName &mapName);
  Sky(class Arena *parent, const SkyInitializer &initializer);
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
};

}
