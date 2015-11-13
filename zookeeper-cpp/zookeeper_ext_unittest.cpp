#include "zookeeper.hpp"
#include "zookeeper_ext.hpp"
#include "zookeeper_error.hpp"
#include <gtest/gtest.h>
#include "zookeeper_unittest_helper.hpp"

using namespace zookeeper;
using namespace testing;

TEST_F(ZooKeeperTest, RecursiveCreate) {
  EXPECT_EQ(RecursiveCreate(zk, "/a"), "/a");
  RecursiveCreate(zk, "/a/b/c");
}

TEST_F(ZooKeeperTest, RecursiveCreateWithBadArgument) {
  EXPECT_THROW(RecursiveCreate(zk, ""), ZooException);
  EXPECT_EQ(RecursiveCreate(zk, "/"), "/");
  EXPECT_THROW(RecursiveCreate(zk, "/a/b/"), ZooException);
  EXPECT_TRUE(zk.Exists("/a/b"));

  zk.Delete("/a/b");
  zk.Delete("/a");
}


