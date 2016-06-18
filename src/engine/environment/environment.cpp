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
#include "environment.hpp"

namespace sky {

/**
 * EnvironmentURL.
 */
optional<std::string> getEnvironmentFile(const EnvironmentURL &url) {
  return optional<std::string>();
}

/**
 * Environment.
 */

void Environment::loadMap() {
  loadProgress = 0;
  map.emplace(); // stub
  loadProgress = 1;
}

void Environment::loadGraphics() {
  loadProgress = 0;
  graphics.emplace(); // stub
  loadProgress = 1;
}

void Environment::loadScripts() {
  loadProgress = 0;
  scripts.emplace(); // stub
  loadProgress = 1;
}

Environment::Environment(const EnvironmentURL &url) :
    url(url),
    loadProgress(0) {
  workerThread = std::thread([&]() { loadMap(); });
}

Environment::Environment() :
    Environment("NULL") {}

Environment::~Environment() {
  workerThread.join();
}

void Environment::loadMore(
    const bool needGraphics, const bool needScripts) {
  if (loadingIdle()) {
    workerThread = std::thread([&]() {
      if (needGraphics) loadGraphics();
      if (needScripts) loadScripts();
    });
  }
}

bool Environment::loadingErrored() const {
  return loadError;
}

bool Environment::loadingIdle() const {
  return workerThread.joinable();
}

float Environment::loadingProgress() const {
  return loadProgress;
}

Map const *Environment::getMap() const {
  return map.get_ptr();
}

}
