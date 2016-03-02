#include "multiplayershared.h"

/**
 * MultiplayerShared.
 */
MultiplayerShared::MultiplayerShared(
    const std::string &serverHostname,
    const unsigned short serverPort) :
    host(tg::HostType::Client),
    server(nullptr),
    telegraph(sky::ServerPacketPack(), sky::ClientPacketPack()),
    pingCooldown(5),
    triedConnection(false),
    disconnecting(false),
    disconnectTimeout(1) {
  host.connect(serverHostname, serverPort);
}

void MultiplayerShared::transmitServer(const sky::ClientPacket &packet) {
  if (server) telegraph.transmit(host, server, packet);
}

/**
 * MultiplayerMode.
 */

MultiplayerMode::Style::Style() :
    scoreOverlayDims{1330, 630},
    chatPos(20, 850),
    messageLogPos(20, 840),
    readyButtonPos(lobbyChatWidth + 10, lobbyTopMargin),
    scoreOverlayTopMargin(100),
    lobbyPlayersOffset(1250),
    lobbyTopMargin(205),
    lobbyChatWidth(1250),
    gameChatWidth(500),
    lobbyFontSize(40),
    playerSpecColor(255, 255, 255),
    playerJoinedColor(0, 255, 0),
    readyButtonActiveDesc("ready!"),
    readyButtonDeactiveDesc("cancel") { }

MultiplayerMode::MultiplayerMode(
    ClientShared &shared,
    MultiplayerShared &mShared) :
    shared(shared),
    mShared(mShared) { }
