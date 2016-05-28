#include <gtest/gtest.h>
#include "sky/sky.h"

/**
 * The Sky subsystem operates and networks correctly.
 */
class SkyTest: public testing::Test {
 public:
  sky::Arena arena;
  sky::Sky sky;

  SkyTest() :
      arena(sky::ArenaInit("special arena", "test1", sky::ArenaMode::Lobby)),
      sky(arena, sky::SkyInit("test1")) { }

};

/**
 * ParticipationInput allows players limited authority over a remote game state.
 */
TEST_F(SkyTest, InputTest) {
  arena.connectPlayer("nameless plane");
  auto &player = *arena.getPlayer(0);
  auto &participation = sky.getParticipation(player);

  player.spawn({}, {200, 200}, 0);

  // We can modify position and control state.
  {
    sky::ParticipationInput input;
    input.physical.emplace(sky::PhysicalState({300, 300}, {}, 50, 0));

    sky::PlaneControls controls(participation.getControls());
    controls.doAction(sky::Action::Left, true);
    input.controls = controls;

    participation.applyInput(input);

    ASSERT_EQ(participation.getPlane()->getState().physical.pos.x, 300);
    ASSERT_EQ(participation.getPlane()->getState().physical.rot, 50);
    ASSERT_EQ(participation.getControls().getState<sky::Action::Left>(), true);
  }

}

/**
 * SkyDeltas can be rewritten to respect the authority of a client.
 */
TEST_F(SkyTest, AuthorityTest) {
  arena.connectPlayer("nameless plane");
  auto &player = *arena.getPlayer(0);
  auto &participation = sky.getParticipation(player);

  sky::Arena remoteArena{arena.captureInitializer()};
  sky::Sky remoteSky{remoteArena, sky.captureInitializer()};
  auto &remotePlayer = *remoteArena.getPlayer(0);
  auto &remoteParticip = remoteSky.getParticipation(remotePlayer);

  // Spawn state is of the server's authority.
  {
    ASSERT_EQ(remoteParticip.isSpawned(), false);

    player.spawn({}, {200, 200}, 0);
    auto delta = sky.collectDelta();
    remoteSky.applyDelta(sky.respectAuthority(delta, player));

    ASSERT_EQ(remoteParticip.isSpawned(), true);
  }

  // Position state is of the client's authority.
  {
    ASSERT_EQ(remoteParticip.getPlane()->getState().physical.pos.x, 200);

    sky::ParticipationInput input;
    input.physical.emplace(sky::PhysicalState({300, 300}, {}, 50, 0));
    participation.applyInput(input);

    auto delta = sky.collectDelta();
    remoteSky.applyDelta(sky.respectAuthority(delta, player));

    ASSERT_EQ(remoteParticip.getPlane()->getState().physical.pos.x, 200);
  }

}


