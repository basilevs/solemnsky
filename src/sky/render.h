/**
 * The render subsystem, managing plane animation state and drawing methods.
 */
#ifndef SOLEMNSKY_GRAPHICS_H
#define SOLEMNSKY_GRAPHICS_H

#include <base/resources.h>
#include "ui/control.h"
#include "ui/sheet.h"
#include "flight.h"
#include <SFML/Graphics.hpp>
#include "sky.h"

namespace sky {

namespace detail {
struct AnimSettings {
  // might as well avoid magic values.. not sure if we'll want to refactor
  // this in the future

  static constexpr float
  // how many degrees it rolls to either side
      rollAmount = 30,
  // how quickly it rolls 2 * rollAmount
      rollSpeed = 2;
};

/**
 * Plain data structure with all the animation state of a plane, alive or not.
 * This is derived from the PlaneState over time after having been thrust
 * into existence at the joining of a plane into the sky, reset to default
 * at each respawn. It is is not necessary to sync this over the network.
 */
struct PlaneAnimState {
  PlaneAnimState();

  Angle roll;
  bool orientation;

  float flipState; // these two values contribute to the roll
  float rollState;

  /**
   * Mutating state.
   */
  void tick(class Plane *parent, const float delta);
  void reset(); // when the plane respawns
  // death is animated in the a normal course of events, but respawning changes
  // everything anew
};
}

class Render : public Subsystem {
private:
  Sky *sky;
  std::map<int, detail::PlaneAnimState> animState;
  const ui::SpriteSheet sheet;

  /**
   * Render submethods.
   */
  float findView(const float viewWidth,
                 const float totalWidth,
                 const float viewTarget) const;
  void renderPlane(ui::Frame &f, const int pid, Plane &plane);

public:
  Render(Sky *sky);

  /**
   * Renders the game from a view centered on pos.
   */
  void render(ui::Frame &f, const sf::Vector2f &pos);

  /**
   * Subsystem listeners.
   */
  void tick(float delta) override;
  virtual void joinPlane(const PID pid, Plane *plane) override;
  virtual void quitPlane(const PID pid) override;
  virtual void spawnPlane(const PID pid, Plane *plane) override;
  virtual void killPlane(const PID pid, Plane *plane) override;
};

}

#endif //SOLEMNSKY_GRAPHICS_H
