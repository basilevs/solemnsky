/**
 * Things that can happen. Useful for [debug] logging and displays.
 */
#pragma once
#include "util/types.h"
#include "util/printer.h"
#include "arena.h"
#include "protocol.h"
#include <SFML/Network.hpp>

namespace sky {

/**
 * onEvent(const ArenaEvent &) is a subsystem callback denoting some event in
 * the Arena. What seperates an ArenaEvent from another subsystem callback is
 * that is is expected to be logged and should therefore be storable
 * in a unified datatype and printable through Printer.
 */

enum class DisconnectType {
  Graceful, Timeout
};

struct ArenaEvent {
  enum class Type {
    Join, Quit, NickChange, TeamChange, ModeChange
  } type;

 private:
  ArenaEvent(const Type type);

 public:
  ArenaEvent() = delete;

  optional<std::string> name, newName;
  optional<sky::Team> oldTeam, newTeam;
  optional<sky::ArenaMode> mode;

  void print(Printer &p) const;

  static ArenaEvent Join(const std::string &name);
  static ArenaEvent Quit(const std::string &name);
  static ArenaEvent NickChange(const std::string &name,
                               const std::string &newName);
  static ArenaEvent TeamChange(const std::string &name, const sky::Team oldTeam,
                               const sky::Team newTeam);
  static ArenaEvent ModeChange(const sky::ArenaMode mode);
};

}

/**
 * ArenaEvent + server-specific events.
 */

struct ServerEvent {
  // TODO: sky::DisconnectType in ServerEvent::Disconnect

  enum class Type {
    Start, Event, Stop, Connect, Disconnect, RConIn, RConOut
  } type;

 private:
  ServerEvent(const Type type);

 public:
  ServerEvent() = delete;

  optional<Port> port;
  optional<std::string> stringData;
  optional<sky::ArenaEvent> arenaEvent;
  optional<double> uptime;

  void print(Printer &p) const;

  static ServerEvent Start(const Port port,
                           const std::string &name);
  static ServerEvent Event(const sky::ArenaEvent &arenaEvent);
  static ServerEvent Stop(const double uptime);
  static ServerEvent Connect(const std::string &name);
  static ServerEvent Disconnect(const std::string &name);

  static ServerEvent RConIn(const std::string &command);
  static ServerEvent RConOut(const std::string &response);
};

/**
 * ArenaEvent + client-specific events.
 */

struct ClientEvent {
  enum class Type {
    Connect, Event, Disconnect, Chat, Broadcast,
    RConCommand, RConResponse
  } type;

 private:
  ClientEvent(const Type type);

 public:
  ClientEvent() = delete;

  optional<std::string> name, message;
  optional<sf::IpAddress> ipAddr;
  optional<sky::ArenaEvent> arenaEvent;
  optional<double> uptime;
  optional<sky::DisconnectType> disconnect;

  void print(Printer &p) const;

  static ClientEvent Connect(const std::string &name,
                             const sf::IpAddress &ipAddr);
  static ClientEvent Event(const sky::ArenaEvent &arenaEvent);
  static ClientEvent Disconnect(const double uptime,
                                const sky::DisconnectType disconnect);
  static ClientEvent Chat(const std::string &name,
                          const std::string &message);
  static ClientEvent Broadcast(const std::string &message);

  static ClientEvent RConCommand(const std::string &command);
  static ClientEvent RConResponse(const std::string &response);
};

