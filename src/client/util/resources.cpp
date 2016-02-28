#include <assert.h>
#include "resources.h"
#include "util/methods.h"

const static std::vector<ResRecord>
    resRecords{ // edit this side-by-side with resources.h
    {"fonts/Atarian/SF Atarian System.ttf", ResType::Font, false},

    {"render-2d/title.png", ResType::Texture, false},
    {"render-2d/menubackground.png", ResType::Texture, false},
    {"render-2d/credits.png", ResType::Texture, false},
    {"render-2d/lobby.png", ResType::Texture, false},
    {"render-2d/scoring.png", ResType::Texture, false},
    {"render-2d/scoreoverlay.png", ResType::Texture, false},

    {"render-3d/test_1/player_200.png", ResType::Texture, true, 200, 200, 2, 15}
};

const std::string filepathTo(const Res res) {
  const ResRecord &record = recordOf(res);
  return "../../media/" + record.path;
}

const ResRecord &recordOf(const Res res) {
  return resRecords[(unsigned long long int) res];
}

/****
 * ResMan
 */

namespace detail {

ResMan::ResMan() {
  assert(resRecords.size() == (size_t) Res::LAST);
}

void ResMan::loadRes() {
  if (initialized) return;

  std::string resCount = std::to_string((int) Res::LAST);
  std::string progress;
  appLog("Loading resources ...", LogOrigin::App);

  for (Res res = Res::Font; res < Res::LAST;
       res = (Res) (((int) res) + 1)) {
    const ResRecord &record(recordOf(res));
    progress = " ... (" + std::to_string((int) res + 1) +
        " of " + resCount + ")";

    switch (record.type) {
      case ResType::Font: {
        appLog("Loading font " + record.path + progress, LogOrigin::App);
        sf::Font font;
        font.loadFromFile(filepathTo(res));
        fonts.emplace((int) res, std::move(font));
        break;
      }
      case ResType::Texture: {
        appLog("Loading texture " + record.path + progress,
               LogOrigin::App);

        sf::Texture texture;
        texture.loadFromFile(filepathTo(res));
        textures.emplace((int) res, std::move(texture));
      }
    }
  }

  appLog(std::to_string(fonts.size()) + " total fonts available.",
         LogOrigin::App);
  appLog(std::to_string(textures.size()) + " total textures available.",
         LogOrigin::App);
  appLog("Finished loading resources.", LogOrigin::App);

  initialized = true;
}

const sf::Texture &ResMan::recallTexture(Res res) {
  assert(initialized);
  const ResRecord &record(recordOf(res));
  if (record.type == ResType::Texture)
    return textures.at((int) res);
  else {
    appErrorLogic(record.path + " is not a texture.");
  }
}

const sf::Font &ResMan::recallFont(Res res) {
  assert(initialized);
  const ResRecord &record(recordOf(res));
  if (record.type == ResType::Font)
    return fonts.at((int) res);
  else {
    appErrorLogic(record.path + " is not a font.");
  }
}

}

void loadResources() { detail::resMan.loadRes(); }

const sf::Texture &textureOf(Res res) {
  return detail::resMan.recallTexture(res);
}

const sf::Font &fontOf(Res res) {
  return detail::resMan.recallFont(res);
}
