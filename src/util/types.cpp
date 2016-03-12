#define _USE_MATH_DEFINES // for M_PI
#include "types.h"
#include <cmath>
#include "methods.h"

/**
 * Cooldown.
 */

void Cooldown::reset() { cooldown = period; }

void Cooldown::prime() { cooldown = 0; }

bool Cooldown::cool(const float delta) {
  cooldown = std::max(0.0f, cooldown - delta);
  return cooldown == 0;
}

Cooldown::Cooldown(const float period) : period(period), cooldown(period) { }

/**
 * Clamped.
 */

Clamped::Clamped() : value(0) { }

Clamped::Clamped(const float value) :
    value(clamp(0.0f, 1.0f, value)) { }

Clamped &Clamped::operator=(const float x) {
  return operator=(Clamped(x));
}

Clamped &Clamped::operator+=(const float x) {
  value = std::min(1.0f, value + x);
  return *this;
}

Clamped &Clamped::operator-=(const float x) {
  value = std::max(0.0f, value - x);
  return *this;
}

/**
 * Movement.
 */
float movementValue(const Movement movement) {
  switch (movement) {
    case Movement::Up:
      return 1;
    case Movement::Down:
      return -1;
    case Movement::None:
      return 0;
  }
}

/**
 * Cyclic.
 */

Cyclic::Cyclic(const float min, const float max) :
    min(min), max(max), value(min) { }

Cyclic::Cyclic(const float min, const float max, const float value) :
    min(min), max(max), value(cyclicClamp(min, max, value)) { }

Cyclic &Cyclic::operator=(const float x) {
  value = cyclicClamp(min, max, x);
  return *this;
}

Cyclic &Cyclic::operator+=(const float x) {
  value = cyclicClamp(min, max, value + x);
  return *this;
}

Cyclic &Cyclic::operator-=(const float x) {
  value = cyclicClamp(min, max, value - x);
  return *this;
}

bool cyclicApproach(Cyclic &x, const float target, const float amount) {
  const float distance = cyclicDistance(x, target);
  x += sign(distance) * std::min(amount, std::abs(distance));
  return amount < distance;
}

float cyclicDistance(const Cyclic x, const float y) {
  const float range = x.max - x.min;
  const float diffUp = Cyclic(0, x.max - x.min, y - x);
  return (diffUp < range / 2) ? diffUp : diffUp - range;
}

/**
 * Angle.
 */

Angle::Angle(const float x) : value(0, 360, x) { }

Angle::Angle(const sf::Vector2f &vec) :
    Angle(toDeg((float) atan2(vec.y, vec.x))) { }

Angle &Angle::operator=(const float x) {
  this->value = x;
  return *this;
}

Angle &Angle::operator+=(const float x) {
  this->value += x;
  return *this;
}

Angle &Angle::operator-=(const float x) {
  this->value -= x;
  return *this;
}

sf::Vector2f Angle::toVector() {
  const float rad = toRad(value);
  return sf::Vector2f((float) cos(rad), (float) sin(rad));
}
