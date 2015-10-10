#include "zookeeper.hpp"
#include "zookeeper_error.hpp"
#include <gtest/gtest.h>
#include <stdlib.h>
#include "zookeeper_mock.hpp"

using namespace zookeeper;
using testing::AtLeast;

const std::string ZOOKEEPER_DAEMON_PATH = "/Volumes/home/zengxg/projects/zookeeper-3.4.6/bin/zkServer.sh";

static void StartZookeeper() {
  auto cmd = ZOOKEEPER_DAEMON_PATH + " start";
  system(cmd.c_str());
}

static void StopZookeeper() {
  auto cmd = ZOOKEEPER_DAEMON_PATH + " stop";
  system(cmd.c_str());
}

struct ZooKeeperTest : testing::Test {
  MockZooWatcher watcher;
  ZooKeeper zk;

  ZooKeeperTest()
  : zk("localhost:2181", &watcher) {
    sleep(2);
    EXPECT_CALL(watcher, OnConnected());
  }
};

struct ZooKeeperTestNoWatch : testing::Test {
  ZooKeeper zk;

  ZooKeeperTestNoWatch()
  : zk("localhost:2181") {
    while (!zk.is_connected()) {
      sleep(1);
    }
  }
};

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

TEST(ZooKeeper, WatchForConnected) {
  MockZooWatcher watcher;
  EXPECT_CALL(watcher, OnConnecting()).Times(0);
  EXPECT_CALL(watcher, OnConnected()).Times(1);

  ZooKeeper zk("localhost:2181", &watcher);
  sleep(1);
}

TEST(ZooKeeper, WatchForConnecting) {
  MockZooWatcher watcher;

  testing::InSequence s;
  EXPECT_CALL(watcher, OnConnected());
  EXPECT_CALL(watcher, OnConnecting());
  EXPECT_CALL(watcher, OnConnected());

  ZooKeeper zk("localhost:2181", &watcher);
  sleep(1);

  StopZookeeper();
  StartZookeeper();

  while (!zk.is_connected()) {
    sleep(1);
  }
}

TEST_F(ZooKeeperTestNoWatch, CreateThenDelete) {
  EXPECT_EQ(zk.Create("/test"), "/test");
  EXPECT_TRUE(zk.Exists("/test"));

  EXPECT_NO_THROW(zk.Delete("/test"));
  EXPECT_FALSE(zk.Exists("/test"));

  EXPECT_THROW(zk.Delete("/test"), ZooException);
}

TEST_F(ZooKeeperTestNoWatch, DeleteNodeWithChildren) {
  zk.Create("/test");
  zk.Create("/test/test");

  EXPECT_THROW(zk.Delete("/test"), ZooException);
  zk.Delete("/test/test");
  zk.Delete("/test");

  EXPECT_FALSE(zk.Exists("/test"));
}

TEST_F(ZooKeeperTestNoWatch, CreateSequence) {
  auto path = zk.Create("/test", "", ZOO_SEQUENCE);
  EXPECT_NE(path, "/test");

  zk.Delete(path);
}

TEST_F(ZooKeeperTestNoWatch, CreateThenGet) {
  zk.Create("/test");
  EXPECT_EQ(zk.Get("/test"), "");
  zk.Delete("/test");

  zk.Create("/test", "abcdefghijk");
  EXPECT_EQ(zk.Get("/test"), "abcdefghijk");
  zk.Delete("/test");
}

TEST_F(ZooKeeperTestNoWatch, SetThenGet) {
  zk.Create("/test");

  zk.Set("/test", "abcdefghijk");
  EXPECT_EQ(zk.Get("/test"), "abcdefghijk");

  zk.Delete("/test");
}

TEST_F(ZooKeeperTestNoWatch, GetNodeThatNotExists) {
  EXPECT_THROW(zk.Get("/node_that_not_exists"), ZooException);
}

TEST_F(ZooKeeperTestNoWatch, GetChildren) {
  zk.Create("/parent");

  EXPECT_TRUE(zk.GetChildren("/parent").empty());

  zk.Create("/parent/a");
  zk.Create("/parent/b");
  EXPECT_EQ(zk.GetChildren("/parent"), std::vector<std::string>({"a", "b"}));

  zk.Delete("/parent/a");
  zk.Delete("/parent/b");
  zk.Delete("/parent");
}

