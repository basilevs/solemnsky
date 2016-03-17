/**
 * The modal multiplayer UI expressed in the form of MultiplayerViews for the
 * arena lobby, game, and scoring screen.
 */
#ifndef SOLEMNSKY_MULTIPLAYERUI_H_H
#define SOLEMNSKY_MULTIPLAYERUI_H_H

#include "multiplayershared.h"
#include "client/subsystem/render.h"

/**
 * In the lobby, we can talk to people, see teams, change teams, and vote for
 * the game to start.
 */
class MultiplayerLobby: public MultiplayerView {
 private:
  ui::Button joinButton;
  ui::Button spectateButton;
  ui::TextEntry chatInput;

  std::vector<ui::Control *> myControls;

 public:
  MultiplayerLobby(ClientShared &shared, MultiplayerConnection &connection);

  void tick(float delta) override;
  void render(ui::Frame &f) override;
  bool handle(const sf::Event &event) override;
  void reset() override;
  void signalRead() override;
  void signalClear() override;

  void onPacket(const sky::ServerPacket &packet) override;
  void onChangeSettings(const SettingsDelta &settings) override;
};

/**
 * In the game, the rendered representation of the game is the central
 * point of the screen; peripheries include a chat / message interface, and a
 * score screen you can call up with tab.
 */
class MultiplayerGame: public MultiplayerView {
 private:
  sky::RenderSystem renderSystem;

 public:
  MultiplayerGame(ClientShared &shared, MultiplayerConnection &connection);

  void tick(float delta) override;
  void render(ui::Frame &f) override;
  bool handle(const sf::Event &event) override;
  void signalRead() override;
  void signalClear() override;

  void onPacket(const sky::ServerPacket &packet) override;
  void onChangeSettings(const SettingsDelta &settings) override;
};

/**
 * At the scoring screen, we display the score and players talk about how
 * they were carrying their team.
 */
class MultiplayerScoring: public MultiplayerView {
 private:

 public:
  MultiplayerScoring(ClientShared &shared, MultiplayerConnection &connection);

  void tick(float delta) override;
  void render(ui::Frame &f) override;
  bool handle(const sf::Event &event) override;
  void signalRead() override;
  void signalClear() override;

  void onPacket(const sky::ServerPacket &packet) override;
  void onChangeSettings(const SettingsDelta &settings) override;
};

#endif //SOLEMNSKY_MULTIPLAYERUI_H_H
