/**
 * Here we define an infrastructure for sending / receiving packets with
 * various transmission control strategies.
 *
 * Yeah, Telegraph. That's what I'm calling it. You can't argue with the Latin.
 * At least I didn't call it SemaphoreLine or something.
 */

#ifndef SOLEMNSKY_TELEGRAPHY_H
#define SOLEMNSKY_TELEGRAPHY_H

#include "pack.h"
#include <SFML/Network.hpp>

namespace tg {

using IpAddress = sf::IpAddress;

/**
 * Transmission strategy: describes the system controls a
 * particular transmission. For instance, a chat packet needs ordering and
 * arrival verification, but a HUD update packet shouldn't take up extra
 * bandwidth to verify arrival or ordering because after 100 ms it's worthless.
 */
struct Strategy {
  enum class Control {
    None, // just send it
    Arrival, // make sure they get there
    Ordering // make sure they get there in their original ordering
  };

  Strategy(const Control = Control::None,
           const optional<double> timeout = {});

  Control control;
  optional<double> timeout;
  // if it takes longer than this value it's no longer profitable for it to
  // arrive at all (applies to non-None controls)
};

/**
 * Reception. Parameter / reference holder for Telegraph::receive() callback.
 */
template<typename T>
struct Reception {
  Reception(const T &value,
            const IpAddress &address) :
      value(value),
      address(address) { }

  const T &value;
  const IpAddress &address;
};

namespace detail {
/**
 * Internal manager for transmitting / receiving a Packet buffer
 * and associated strategy-related flags.
 */
class Wire {
private:
  Packet *buffer;
  // int orderFlag; etc etc

public:
  Wire() = delete;
  Wire(Packet *buffer);

  bool receive(sf::UdpSocket &sock,
               IpAddress &addr,
               unsigned short &port);
  void transmit(sf::UdpSocket &sock,
                const IpAddress &addr,
                const unsigned short port);
};
}

/**
 * Transmission manager of sorts, used both as a listener and as a transmitter.
 * Every UDP connection in our design has one of these at both ends.
 */
template<typename TransmitT, typename ReceiveT>
class Telegraph {
private:
  sf::UdpSocket sock;
  ReceiveT valueBuffer;
  Packet buffer;
  detail::Wire wire;

  const Pack<TransmitT> transmitRule;
  const Pack<ReceiveT> receiveRule;

public:
  Telegraph(const unsigned short port,
            const Pack<TransmitT> &transmitRule,
            const Pack<ReceiveT> &receiveRule) :
      port(port),
      transmitRule(transmitRule),
      receiveRule(receiveRule),
      buffer(),
      wire(&buffer) {
    sock.setBlocking(false);
    sock.bind(port);
  }

  const unsigned short port;

  /**
   * Receptions and transmissions.
   */
  void transmit(const TransmitT &value,
                const IpAddress &address,
                const unsigned short port,
                const Strategy &strategy = Strategy());
  void receive(std::function<void(Reception<ReceiveT> &&)> onReceive);
};

template<typename TransmitT, typename ReceiveT>
void Telegraph<TransmitT, ReceiveT>
::transmit(const TransmitT &value,
           const IpAddress &address, const unsigned short port,
           const Strategy &strategy) {
  packInto(transmitRule, value, buffer);
  // wire.header = blah blah depending on strategy
  wire.transmit(sock, address, port);
}

template<typename TransmitT, typename ReceiveT>
void Telegraph<TransmitT, ReceiveT>
::receive(std::function<void(Reception<ReceiveT> &&)> onReceive) {
  static IpAddress address;
  static unsigned short port;
  while (wire.receive(sock, address, port)) {
    unpackInto(receiveRule, buffer, valueBuffer);
    onReceive(Reception<ReceiveT>(valueBuffer, address));
    // potentially respond to server or don't immediately add to cue
    // according to strategy in use
  }
}

}

#endif //SOLEMNSKY_TELEGRAPHY_H