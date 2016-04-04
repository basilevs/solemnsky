/**
 * The ubiquitous checkbox.
 */
#pragma once
#include "ui/control.h"
#include "button.h"

namespace ui {

class Checkbox: public ui::Control {
 private:
  Button button;
  bool value;

 public:
  typedef Button::Style Style;

  Checkbox() = delete;
  Checkbox(const Style &style,
           const sf::Vector2f &pos);

  /**
   * Control interface.
   */
  void tick(float delta) override;
  void render(Frame &f) override;
  bool handle(const sf::Event &event) override;
  void signalRead() override;
  void signalClear() override;
  void reset() override;

  /**
   * Custom.
   */
  void setValue(const bool newValue);
  bool getValue() const;

  void setDescription(const optional<std::string> &description);

  bool &clickSignal;
};

}
