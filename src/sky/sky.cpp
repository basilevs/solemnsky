#include "sky.h"

namespace sky {

void Sky::joinPlane(const PID pid, const PlaneTuning tuning) {
  planes.emplace(std::pair<int, Plane>(pid, Plane(this, tuning)));
}

void Sky::quitPlane(const PID pid) {
  planes.erase(pid);
}

Plane &Sky::getPlane(const PID pid) {
  return planes.at(pid);
}

void Sky::handle(const PID pid, const PlayerInput input) {

}

void Sky::tick(float delta) {
  physics.tick(delta);
  for (auto elem : planes) {
    Plane &plane = elem.second;
    plane.readFromBody();
    plane.tick(delta);
    plane.writeToBody();
  }
}

/****
 * Physics settings.
 */

Physics::Settings mySettings() {
  Physics::Settings settings{};
  settings.velocityIterations = 7; // etc
  return settings;
};

Sky::Sky() : physics(mySettings()) {
  appLog(LogType::Notice, "Opening a new sky."); // very important logging
}

Sky::~Sky() {
  appLog(LogType::Notice, "Closing an old sky.");
}

}
