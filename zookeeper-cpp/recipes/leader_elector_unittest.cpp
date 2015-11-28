#include <gtest/gtest.h>
#include "leader_elector.h"
#include "leader_elector_mock.h"
#include "zookeeper-cpp/zookeeper_unittest_helper.hpp"

using namespace testing;
using namespace zookeeper;

TEST(LeaderElector, Join) {
  MockLeaderElectorHandler handler;
  EXPECT_CALL(handler, TakeLeadership());

  LeaderElector zk("127.0.01:2181", "/test_service", &handler);
  zk.Join();
  sleep(2);
}

TEST(LeaderElector, RevokeLeadershipIfDisconnected) {
  MockLeaderElectorHandler handler;

  InSequence s;

  EXPECT_CALL(handler, TakeLeadership());
  LeaderElector zk("127.0.01:2181", "/test_service", &handler);
  zk.Join();
  sleep(2);

  EXPECT_CALL(handler, RevokeLeadership());
  StopZookeeper();
  sleep(2);

  EXPECT_CALL(handler, TakeLeadership());
  StartZookeeper();
  sleep(5);
}

TEST(LeaderElector, LeaderSwitch) {
  MockLeaderElectorHandler handler1;
  MockLeaderElectorHandler handler2;

  EXPECT_CALL(handler1, TakeLeadership());
  LeaderElector zk1("127.0.01:2181", "/test_service", &handler1);
  zk1.Join();

  EXPECT_CALL(handler2, LeadershipChanged(_));
  LeaderElector zk2("127.0.01:2181", "/test_service", &handler2);
  zk2.Join();

  sleep(2);

  EXPECT_CALL(handler2, TakeLeadership());
  zk1.Leave();

  sleep(2);

  EXPECT_CALL(handler1, LeadershipChanged(_));
  zk1.Join();

  sleep(5);
}

