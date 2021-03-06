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
 * Set of static information that surrounds a Sky.
 */
#pragma once
#include "util/types.hpp"
#include "visuals.hpp"
#include "mechanics.hpp"
#include "map.hpp"
#include "util/threads.hpp"
#include "util/archive.hpp"
#include "engine/types.hpp"

namespace sky {

/**
 * Holder and asynchronous loader for pieces of static information extracted
 * from a .sky file, used to instantiate / add to the functionality /
 * display a Sky -- geometry data, scripts, and graphics resources.
 */
class Environment {
 private:
  // Associated archive and filepath -- this makes no sense if the environment is NULL.
  fs::path archivePath;
  Archive fileArchive;

  // State.
  bool workerRunning;
  bool loadError;
  float loadProgress;
  optional<Map> map;
  optional<Visuals> visuals;
  optional<Mechanics> mechanics;

  std::thread workerThread;

  // Canonical logging messages.
  enum class Component { Map, Mechanics, Visuals };
  static std::string describeComponent(const Component c);
  static std::string describeComponentLoading(const Component c);
  static std::string describeComponentDone(const Component c);
  static std::string describeComponentLoadingNull(const Component c);
  static std::string describeComponentMissing(const Component c);
  static std::string describeComponentMalformed(const Component c);

  // Loading subroutines -- they expect fileArchive to be loaded.
  void loadMap(const fs::path &path);
  void loadMechanics(const fs::path &path);
  void loadVisuals(const fs::path &path);

  // Null loading subroutines.
  void loadNullMap();
  void loadNullMechanics();
  void loadNullVisuals();

 public:
  Environment(const EnvironmentURL &url);
  // The null environment, useful for testing and sandboxes.
  // The default ctor is equilivent to supplying a URL of "NULL".
  Environment();
  ~Environment();

  const EnvironmentURL url;

  // Loading.
  void loadMore(const bool needVisuals, const bool needMechanics);
  void joinWorker();

  // Load status.
  bool loadingErrored() const;
  bool loadingIdle() const;
  float loadingProgress() const;

  // Accessing loaded resources. nullptr if they aren't loaded.
  Map const *getMap() const;
  Visuals const *getVisuals() const;
  Mechanics const *getMechanics() const;

};

}

