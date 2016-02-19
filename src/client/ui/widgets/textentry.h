/**
 * Generic, highly customizable text entry, with cursor and mouse interaction
 * and fancy stuff like that.
 */
#ifndef SOLEMNSKY_TEXTENTRY_H
#define SOLEMNSKY_TEXTENTRY_H

#include "util/types.h"
#include "client/ui/control.h"

namespace ui {

class TextEntry : public Control {
 public:
  struct Style {
    sf::Color inactiveBgColor{142, 183, 191}, // background when inactive
        hotBgColor{162, 203, 211}, // background when active (hot)
        focusedBgColor{255, 255, 255},
        descriptionColor{100, 100, 100}, // description text
        textColor{0, 0, 0}; // text
    sf::Vector2f dimensions{500, 40};
    float cursorWidth = 5;
    float sidePadding = 10;
    int fontSize = 30;
    float heatRate = 10;

    Style() { }
  } style;

 private:
  Clamped heat;

  float scroll;
  int cursor;

  optional<sf::Event> pressedKeyboardEvent;
  Cooldown repeatActivate;
  Cooldown repeatCooldown;

  sf::FloatRect getBody();

  bool handleKeyboardEvent(const sf::Event &event);

 public:

  TextEntry() = delete;
  TextEntry(const sf::Vector2f &pos,
            const std::string &description = "",
            const bool persistent = false,
            const Style &style = {});

  bool persistent; // text is persistent, doesn't reset on focus change / entry
  // and the displayed description is the contents
  sf::Vector2f pos;

  /**
   * Control implementation.
   */
  virtual void tick(float delta) override;
  virtual void render(Frame &f) override;
  virtual bool handle(const sf::Event &event) override;
  virtual void signalRead() override;
  virtual void signalClear() override;

  /**
   * UI methods.
   */
  std::string contents;
  std::string description;
  void reset(); // reset animations
  void focus();
  void unfocus();

  /**
   * UI state and signals, read-only.
   */
  bool isHot;
  bool isFocused;
  optional<std::string> inputSignal;
};

}

#endif //SOLEMNSKY_TEXTENTRY_H