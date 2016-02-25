/**
 * Data structures / simulation and box2d integration for planes.
 */
#ifndef SOLEMNSKY_FLIGHT_H
#define SOLEMNSKY_FLIGHT_H

#include <Box2D/Box2D.h>
#include "physics.h"
#include "util/types.h"
#include "telegraph/pack.h"

namespace sky {

class Sky;

/**
 * Tuning values describing how a plane flies.
 */
struct PlaneTuning {
  PlaneTuning() { }

  sf::Vector2f hitbox{110, 60}; // x axis parallel with flight
  float maxHealth = 10;

  struct Energy {
    float thrustDrain = 1;
    float recharge = 0.5;
    float laserGun = 0.3;
  } energy;

  struct Stall {
    // mechanics when stalled
    float maxRotVel = 200, // how quickly we can turn, (deg / s)
        maxVel = 300, // our terminal velocity (px / s)
        thrust = 500, // thrust acceleration (ps / s^2)
        damping = 0.8; // how quickly we approach our terminal velocity
    float threshold = 130; // the minimum airspeed that we need to enter flight
  } stall;

  struct Flight {
    // mechanics when not stalled
    float maxRotVel = 180,
        airspeedFactor = 330,
        throttleInfluence = 0.6,
        throttleEffect = 0.3,
        gravityEffect = 0.6,
        afterburnDrive = 0.9,
        leftoverDamping = 0.3;
    float threshold = 110; // the maximum airspeed that we need to enter stall
  } flight;

  float throttleSpeed = 1.5;
};

struct PlaneTuningPack: public tg::ClassPack<PlaneTuning> {
  PlaneTuningPack();
};

/**
 * State specific to a plane that is spawned, everything in here is expected
 * to change very frequently, while it exists anyway.
 */
struct PlaneVital {
  PlaneVital(); // for packing
  PlaneVital(const PlaneTuning &tuning,
             const sf::Vector2f &pos,
             const float rot);

  /**
   * Data.
   * Clamped values should have the same bounds in all instantiations.
   */
  Clamped rotCtrl; // controls
  Movement throtCtrl;

  sf::Vector2f pos, vel; // physical values
  Angle rot;
  float rotvel;

  bool stalled; // flight mechanics
  Clamped afterburner;
  sf::Vector2f leftoverVel;
  Clamped airspeed, throttle;

  Clamped energy, health; // game mechanics

  /**
   * Helper methods.
   */
  float forwardVelocity() const;
  float velocity() const;

  // returns true if energy was drawn
  bool requestDiscreteEnergy(const float reqEnergy);
  // returns the fraction of the requested energy that was drawn
  float requestEnergy(const float reqEnergy);
};

struct PlaneVitalPack: public tg::ClassPack<PlaneVital> { PlaneVitalPack(); };

/**
 * A plane, expressed in a simple (copyable etc.) struct.
 */
struct Plane {
  Plane();
  Plane(const PlaneTuning &tuning, const PlaneVital &vital);

  PlaneTuning tuning;
  PlaneVital vital; // exists <=> plane is spawned
};

struct PlanePack: public tg::ClassPack<Plane> {
  PlanePack();
};

/**
 * Handle for a plane, manages the interface with box2d and in doing so holds
 * a pointer, making it non-copyable.
 */
class PlaneHandle {
 private:
  sky::Sky *engine;
  Physics *physics;
  b2Body *body;

  /*
   * State mutation.
   */
  friend class Sky;

  void writeToBody();
  void readFromBody();
  void tick(float d);

 public:
  PlaneHandle(Sky *engine,
              const PlaneTuning &tuning,
              const sf::Vector2f pos,
              const float rot);
  ~PlaneHandle();

  PlaneHandle(PlaneHandle &&);
  PlaneHandle(const PlaneHandle &) = delete;
  PlaneHandle &operator=(const PlaneHandle &) = delete;

  Plane state{};
};
}

#endif //SOLEMNSKY_FLIGHT_H
