/**
 * The place where the whole client converges.
 */
#pragma once
#include "sky/participation.h"
#include "ui/control.h"
#include "settings.h"

class Client;

enum class PageType {
  Home, Listing, Settings
};

/**
 * Some UI state of our client; the various transitions and menu modes.
 */
class ClientUiState {
private:
  friend class Client;

  void focusGame();
  void blurGame();
  void focusPage(PageType page);
  void blurPage();

  void tick(float delta);

public:
  ClientUiState();

  PageType focusedPage;
  bool pageFocusing;
  bool gameFocusing;

  Clamped pageFocusFactor;
  Clamped gameFocusFactor;

  bool pageFocused() const;
  bool menuFocused() const;
  bool gameFocused() const;
};

/**
 * This is the shared core of the client, which all sub-elements refer to. It
 * has globally useful factors such as global settings, a unique_ptr to the Game
 * currently active, and UI methods that influence the whole Client.
 *
 * This should stay as small as possible.
 */
struct ClientShared {
private:
  Client &client;

 public:
  ClientShared(ui::AppState &appState, Client &client);

  // AppState.
  ui::AppState &appState;

  // State.
  Settings settings;
  double uptime;
  std::unique_ptr<class Game> game;
  ClientUiState ui;

  // query key bindings
  optional<std::pair<sky::Action, bool>> triggerSkyAction(
      const sf::Event &event) const;
  optional<std::pair<ClientAction, bool>> triggerClientAction(
      const sf::Event &event) const;

  // Relays to client. This seems to be useful enough to justify violating DRY.
  void beginGame(std::unique_ptr<Game> &&game);
  void blurGame();
  void focusGame();
  void exitGame();

  void focusPage(const PageType type);
  void blurPage();

  void changeSettings(const SettingsDelta &settings);
};
