#include "zookeeper.hpp"
#include "zookeeper_error.hpp"
#include <gtest/gtest.h>

using namespace zookeeper;

TEST(ZooKeeper, ConstructWithBadArgument) {
  EXPECT_THROW(ZooKeeper zk(""), ZooException);
  EXPECT_THROW(ZooKeeper zk("localhost"), ZooException);
}

TEST(ZooKeeper, ConstructAndConnected) {
  ZooKeeper zk("localhost:2181");
  sleep(1);
  EXPECT_TRUE(zk.is_connected());
}

TEST(ZooKeeper, Exists) {
  ZooKeeper zk("localhost:2181");
  sleep(1);
  EXPECT_TRUE(zk.is_connected());

  EXPECT_FALSE(zk.Exists("/abc"));
}

