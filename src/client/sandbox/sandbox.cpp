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
#include "client/elements/style.hpp"
#include "sandbox.hpp"

/**
 * SandboxCommand.
 */

SandboxCommand::SandboxCommand(const std::string &input) {
  type = Type::Start;
  mapName.emplace("ball_asteroids");
}

/**
 * Sandbox.
 */

void Sandbox::startHandle() {
  skyHandle.start();
  if (!skyHandle.loadingErrored()) {
    skyRender.emplace(shared, resources, arena, skyHandle.sky.get());
    status = "running";
  }
}

void Sandbox::stopHandle() {
  status = "stopped";
  skyRender.reset();
  skyHandle.stop();
}

void Sandbox::runCommand(const SandboxCommand &command) {
  switch (command.type) {
    case SandboxCommand::Type::Start: {
      arena.applyDelta(sky::ArenaDelta::MapChange(
          command.mapName.get()));
      startHandle();
      break;
    }
    case SandboxCommand::Type::Stop: {
      stopHandle();
      break;
    }
  }

}

Sandbox::Sandbox(ClientShared &state) :
    Game(state, "sandbox"),
    arena(sky::ArenaInit("sandbox", "ball_funnelpark")),
    skyHandle(arena, sky::SkyHandleInit()),
    debugView(arena, skyHandle),
    commandEntry(references, style.base.normalTextEntry, style.multi.chatPos) {
  arena.connectPlayer("offline player");
  player = arena.getPlayer(0);
  stopHandle();
  areChildren({&commandEntry});
}

/**
 * Game interface.
 */

void Sandbox::onChangeSettings(const SettingsDelta &settings) {
  ClientComponent::onChangeSettings(settings);
  if (skyRender) skyRender->onChangeSettings(settings);
  if (settings.nickname) player->getNickname() = *settings.nickname;
}

void Sandbox::onBlur() {

}

void Sandbox::onFocus() {

}

void Sandbox::doExit() {
  quitting = true;
}

/**
 * Control interface.
 */

void Sandbox::tick(float delta) {
  if (shared.ui.gameFocused()) arena.tick(delta);
  // if this were multiplayer of course we wouldn't have this liberty
  ui::Control::tick(delta);
}

void Sandbox::render(ui::Frame &f) {
  if (skyHandle.isActive()) {
    const auto &plane = skyHandle.sky.get().getParticipation(*player).plane;
    skyRender->render(
        f, plane ?
           plane->getState().physical.pos :
           sf::Vector2f(0, 0));
  }
  ui::Control::render(f);
}

bool Sandbox::handle(const sf::Event &event) {
  if (skyHandle.isActive()) {
    const auto &participation = skyHandle.sky.get().getParticipation(*player);

    if (auto action = shared.triggerClientAction(event)) {
      if (action->first == ClientAction::Spawn
          and action->second
          and !participation.isSpawned()) {
        player->spawn({}, {300, 300}, 0);
        return true;
      }

      if (action->first == ClientAction::Chat
          and action->second
          and !commandEntry.isFocused) {
        commandEntry.focus();
      }
    }

    if (auto action = shared.triggerSkyAction(event)) {
      player->doAction(action->first, action->second);
    }
  }

  return ui::Control::handle(event);
}

void Sandbox::reset() {
  ui::Control::reset();
}

void Sandbox::signalRead() {
  if (const auto signal = commandEntry.inputSignal) {
    runCommand(signal.get());
  }
  ui::Control::signalRead();
}

void Sandbox::signalClear() {
  ui::Control::signalClear();
}

void Sandbox::printDebugLeft(Printer &p) {
  debugView.printArenaReport(p);
}

void Sandbox::printDebugRight(Printer &p) {
  debugView.printSkyReport(p);
}
