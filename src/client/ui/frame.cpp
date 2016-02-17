#include <assert.h>
#include <cmath>
#include "util/methods.h"
#include "frame.h"

namespace ui {
const sf::Color Frame::alphaScaleColor(const sf::Color color) {
  sf::Color newColor(color);
  newColor.a *= alphaStack.top();
  return newColor;
}

Frame::Frame(sf::RenderWindow &window) : window(window) {
  resize();
}

void Frame::resize() {
  sf::Vector2u size = window.getSize();
  float sx(size.x), sy(size.y);
  const float viewAspect = sx / sy;
  static constexpr float targetAspect = 16.0f / 9.0f;

  // set the view
  sf::View view;
  view.setCenter(800, 450);
  if (viewAspect > targetAspect) {
    view.setSize(1600 * (viewAspect / targetAspect), 900);
    float scalar = 900 / sy;
    windowToFrame = sf::Transform().scale(scalar, scalar);
    windowToFrame.translate(-(sx - (sy * targetAspect)) / 2, 0);
  }
  else {
    view.setSize(1600, 900 * (targetAspect / viewAspect));
    float scalar = 1600 / sx;
    windowToFrame = sf::Transform().scale(scalar, scalar);
    windowToFrame.translate(0, -(sy - (sx / targetAspect)) / 2);
  }

  window.setView(view);
}

void Frame::beginDraw() {
  primCount = 0;
  transformStack = std::stack<sf::Transform>({sf::Transform()});
  alphaStack = std::stack<float>({1});
  window.clear(sf::Color::Black);
}

void Frame::endDraw() {
  sf::Vector2u size = window.getSize();
  sf::Vector2f topLeft = windowToFrame.transformPoint(sf::Vector2f(0, 0));
  sf::Vector2f bottomRight = windowToFrame.transformPoint(
      sf::Vector2f(size));

  // overwrite the margins outside of the 1600 x 900 we allow 'ctrl' to draw in
  sf::Color color = sf::Color(20, 20, 20, 255);
  drawRect(topLeft, sf::Vector2f(0, 900), color);
  drawRect(sf::Vector2f(1600, 0), bottomRight, color);
  drawRect(topLeft, sf::Vector2f(1600, 0), color);
  drawRect(sf::Vector2f(0, 900), bottomRight, color);
}

void Frame::pushTransform(const sf::Transform &transform) {
  sf::Transform nextTransform{transformStack.top()};
  nextTransform.combine(transform);
  transformStack.push(nextTransform);
}

void Frame::popTransform() {
  assert(transformStack.size() > 1);
  transformStack.pop();
}

void Frame::withTransform(const sf::Transform &transform,
                          std::function<void()> fn) {
  pushTransform(transform);
  fn();
  popTransform();
}

void Frame::pushAlpha(const float alpha) {
  alphaStack.push(alphaStack.top() * alpha);
}

void Frame::popAlpha() {
  assert(alphaStack.size() > 1);
  alphaStack.pop();
}

void Frame::withAlpha(const float alpha, std::function<void()> fn) {
  pushAlpha(alpha);
  fn();
  popAlpha();
}

void Frame::withAlphaTransform(
    const float alpha, const sf::Transform &transform,
    std::function<void()> fn) {
  pushAlpha(alpha), pushTransform(transform);
  fn();
  popTransform(), popAlpha();
}

void Frame::drawCircle(const sf::Vector2f &pos,
                       const float radius,
                       const sf::Color &color) {
  primCount++;
  sf::CircleShape circle(radius);
  circle.setPosition(pos - sf::Vector2f(radius, radius));
  circle.setFillColor(alphaScaleColor(color));
  window.draw(circle, transformStack.top());
}

void Frame::drawRect(const sf::Vector2f &topLeft,
                     const sf::Vector2f &bottomRight,
                     const sf::Color &color) {
  primCount++;
  sf::RectangleShape rect(bottomRight - topLeft);
  rect.setPosition(topLeft);
  rect.setFillColor(alphaScaleColor(color));
  window.draw(rect, transformStack.top());
}

void Frame::drawText(const sf::Vector2f &pos,
                     const std::vector<std::string> &contents,
                     const int size,
                     const sf::Color &color,
                     const sf::Font &font,
                     const sf::Text::Style &style) {
  float yOffset = 0;

  for (const auto &string : contents) {
    primCount++;
    sf::Text text;
    text.setFont(font);
    text.setPosition(pos + sf::Vector2f(0, yOffset));
    text.setString(string);
    text.setCharacterSize((unsigned int) size);
    text.setColor(alphaScaleColor(color));
    text.setStyle(style);
    window.draw(text, transformStack.top());

    yOffset += textSize(string, size, font).y;
  }
}

void Frame::drawSprite(const sf::Texture &texture,
                       const sf::Vector2f &pos,
                       const sf::IntRect &portion) {
  primCount++;
  sf::Sprite sprite;
  sprite.setTexture(texture);
  sprite.setPosition(pos);
  sprite.setTextureRect(portion);
  sprite.setColor(sf::Color(255, 255, 255, (sf::Uint8) (255 * alphaStack.top())));
  window.draw(sprite, transformStack.top());
}

sf::Vector2f Frame::textSize(const std::string contents, const int size,
                             const sf::Font &font) {
  sf::Text text;
  text.setFont(font);
  text.setString(contents);
  text.setCharacterSize((unsigned int) size);
  const auto bounds = text.getLocalBounds();
  return sf::Vector2f(bounds.width, size); // bounds.height);
}

}
