#include "pack.h"

namespace tg {

BoolPack::BoolPack() : Pack<bool>(
    [](PacketWriter &writer, const bool &value) {
      writer.writeBit(value);
    },
    [](PacketReader &reader, bool &value) -> bool {
      optional<bool> readBit = reader.readBit();
      if (readBit) value = *readBit;
      else return false;
      return true;
    }) { }

StringPack::StringPack() : Pack<std::string>(
    [](PacketWriter &writer, const std::string &value) {
      for (const char x : value)
        writer.writeChar((unsigned char) x);
      writer.writeChar(0);
    },
    [](PacketReader &reader, std::string &value) -> bool {
      value.clear();
      optional<unsigned char> current;
      for (; ;) {
        current = reader.readChar();
        if (!current) return false;
        if (*current == 0) return true;
        value.push_back(*current);
      }
    }) { }

/**
 * Instantiated Pack's.
 */

const Pack<std::string> stringPack = StringPack();

const Pack<optional<std::string>> optStringPack =
    OptionalPack<std::string>(stringPack);

const Pack<bool> boolPack = BoolPack();

const Pack<float> floatPack = BytePack<float>();

const Pack<int> intPack = BytePack<int>();

const Pack<sf::Vector2f> vectorPack =
    ClassPack<sf::Vector2f>(
        MemberRule<sf::Vector2f, float>(floatPack, &sf::Vector2f::x),
        MemberRule<sf::Vector2f, float>(floatPack, &sf::Vector2f::y)
    );

const Pack<Clamped> clampedPack = AssignPack<float, Clamped>(floatPack);

const Pack<Angle> anglePack = AssignPack<float, Angle>(floatPack);

const Pack<Movement> movementPack = EnumPack<Movement>(2);

}
