#include "zookeeper.hpp"
#include "zookeeper_error.hpp"
#include <gtest/gtest.h>
#include <stdlib.h>
#include "zookeeper_mock.hpp"

using namespace zookeeper;
using namespace testing;

const std::string ZOOKEEPER_DAEMON_PATH = "/Volumes/home/zengxg/projects/zookeeper-3.4.6/bin/zkServer.sh";
const std::string ZOOKEEPER_HOSTS = "127.0.0.1:2181";

static void StartZookeeper() {
  auto cmd = ZOOKEEPER_DAEMON_PATH + " start";
  system(cmd.c_str());
}

static void StopZookeeper() {
  auto cmd = ZOOKEEPER_DAEMON_PATH + " stop";
  system(cmd.c_str());
}

static void WaitForConnected(ZooKeeper& zk) {
  while (!zk.is_connected()) {
    sleep(1);
  }
}

/*
struct ZooKeeperTest : testing::Test {
  MockZooWatcher watcher;
  ZooKeeper zk;

  ZooKeeperTest()
  : zk("localhost:2181", &watcher) {
    sleep(2);
    EXPECT_CALL(watcher, OnConnected());
  }
};
*/

struct ZooKeeperTest : Test {
  ZooKeeper zk;

  ZooKeeperTest()
  : zk("localhost:2181") {
    WaitForConnected(zk);
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

TEST_F(ZooKeeperTest, Exists) {
  EXPECT_FALSE(zk.Exists("/abc"));
}

TEST_F(ZooKeeperTest, CreateThenDelete) {
  EXPECT_EQ(zk.Create("/test"), "/test");
  EXPECT_TRUE(zk.Exists("/test"));

  EXPECT_NO_THROW(zk.Delete("/test"));
  EXPECT_FALSE(zk.Exists("/test"));

  EXPECT_THROW(zk.Delete("/test"), ZooException);
}

TEST_F(ZooKeeperTest, DeleteNodeWithChildren) {
  zk.Create("/test");
  zk.Create("/test/test");

  EXPECT_THROW(zk.Delete("/test"), ZooException);
  zk.Delete("/test/test");
  zk.Delete("/test");

  EXPECT_FALSE(zk.Exists("/test"));
}

TEST_F(ZooKeeperTest, CreateSequence) {
  auto path = zk.Create("/test", "", ZOO_SEQUENCE);
  EXPECT_NE(path, "/test");

  zk.Delete(path);
}

TEST_F(ZooKeeperTest, CreateThenGet) {
  zk.Create("/test");
  EXPECT_EQ(zk.Get("/test"), "");
  zk.Delete("/test");

  zk.Create("/test", "abcdefghijk");
  EXPECT_EQ(zk.Get("/test"), "abcdefghijk");
  zk.Delete("/test");
}

TEST_F(ZooKeeperTest, SetThenGet) {
  zk.Create("/test");

  zk.Set("/test", "abcdefghijk");
  EXPECT_EQ(zk.Get("/test"), "abcdefghijk");

  zk.Delete("/test");
}

TEST_F(ZooKeeperTest, GetNodeThatNotExists) {
  EXPECT_THROW(zk.Get("/node_that_not_exists"), ZooException);
}

TEST_F(ZooKeeperTest, GetChildren) {
  zk.Create("/parent");

  EXPECT_TRUE(zk.GetChildren("/parent").empty());

  zk.Create("/parent/a");
  zk.Create("/parent/b");
  EXPECT_EQ(zk.GetChildren("/parent"), std::vector<std::string>({"a", "b"}));

  zk.Delete("/parent/a");
  zk.Delete("/parent/b");
  zk.Delete("/parent");
}

// test for watch change
TEST(ZooKeeperWatch, WatchForConnected) {
  MockZooWatcher watcher;
  EXPECT_CALL(watcher, OnConnecting()).Times(0);
  EXPECT_CALL(watcher, OnConnected()).Times(1);

  ZooKeeper zk("localhost:2181", &watcher);
  sleep(1);
}

TEST(ZooKeeperWatch, WatchForConnecting) {
  MockZooWatcher watcher;

  InSequence s;
  EXPECT_CALL(watcher, OnConnected());
  EXPECT_CALL(watcher, OnConnecting());
  EXPECT_CALL(watcher, OnConnected());

  ZooKeeper zk("localhost:2181", &watcher);
  sleep(1);

  StopZookeeper();
  StartZookeeper();

  WaitForConnected(zk);
}

TEST(ZooKeeperWatch, WatchForCreated) {
  MockZooWatcher watcher;

  EXPECT_CALL(watcher, OnConnected());
  EXPECT_CALL(watcher, OnCreated(StrEq("/test")));

  ZooKeeper zk("localhost:2181", &watcher);
  WaitForConnected(zk);

  EXPECT_FALSE(zk.Exists("/test", true));

  //
  zk.Create("/test");
  zk.Delete("/test");

  sleep(1);
}

TEST(ZooKeeperWatch, WatchForDelete) {
  MockZooWatcher watcher;

  EXPECT_CALL(watcher, OnConnected());
  EXPECT_CALL(watcher, OnDeleted(StrEq("/test")));

  ZooKeeper zk("localhost:2181", &watcher);
  WaitForConnected(zk);

  //
  zk.Create("/test");

  EXPECT_TRUE(zk.Exists("/test", true));

  zk.Delete("/test");

  sleep(1);
}

TEST(ZooKeeperWatch, WatchForChange) {
  MockZooWatcher watcher;

  EXPECT_CALL(watcher, OnConnected());
  EXPECT_CALL(watcher, OnChanged(StrEq("/test")));

  ZooKeeper zk("localhost:2181", &watcher);
  WaitForConnected(zk);

  //
  zk.Create("/test");

  EXPECT_TRUE(zk.Exists("/test", true));
  zk.Set("/test", "new_value");

  zk.Delete("/test");

  sleep(1);
}

TEST(ZooKeeperWatch, WatchSameNodeMultitimes) {
  MockZooWatcher watcher;

  EXPECT_CALL(watcher, OnConnected());
  EXPECT_CALL(watcher, OnCreated(StrEq("/test"))).Times(1);
  EXPECT_CALL(watcher, OnChanged(StrEq("/test"))).Times(1);
  EXPECT_CALL(watcher, OnDeleted(StrEq("/test"))).Times(1);

  ZooKeeper zk("localhost:2181", &watcher);
  WaitForConnected(zk);

  zk.Exists("/test", true);
  zk.Exists("/test", true);

  //
  zk.Create("/test");

  zk.Exists("/test", true);
  zk.Exists("/test", true);
  zk.Set("/test", "abc");

  zk.Exists("/test", true);
  zk.Exists("/test", true);
  zk.Delete("/test");

  sleep(1);
}

TEST(ZooKeeperWatch, ChildrenOperationWontAffectParent) {
  MockZooWatcher watcher;

  EXPECT_CALL(watcher, OnConnected());
  EXPECT_CALL(watcher, OnDeleted(StrEq("/test"))).Times(1);

  ZooKeeper zk("localhost:2181", &watcher);
  WaitForConnected(zk);

  zk.Create("/test");

  zk.Exists("/test", true);
  zk.Create("/test/abc");
  zk.Set("/test/abc", "def");
  zk.Delete("/test/abc");

  zk.Delete("/test");
  sleep(1);
}

TEST(ZooKeeperWatch, GetAndWatchRequireNodeExist) {
  MockZooWatcher watcher;

  EXPECT_CALL(watcher, OnConnected());

  ZooKeeper zk("localhost:2181", &watcher);
  WaitForConnected(zk);

  EXPECT_THROW(zk.Get("/test", true), ZooException);

  zk.Create("/test");
  zk.Set("/test", "abc");
  zk.Delete("/test");
  sleep(1);
}

TEST(ZooKeeperWatch, GetAndWatch) {
  MockZooWatcher watcher;

  EXPECT_CALL(watcher, OnConnected());
  EXPECT_CALL(watcher, OnChanged(StrEq("/test"))).Times(1);
  EXPECT_CALL(watcher, OnDeleted(StrEq("/test"))).Times(1);

  ZooKeeper zk("localhost:2181", &watcher);
  WaitForConnected(zk);

  zk.Create("/test");

  zk.Get("/test", true);
  zk.Set("/test", "abc");

  zk.Get("/test", true);
  zk.Delete("/test");
  sleep(1);
}

TEST(ZooKeeperWatch, GetChildrenAndWatchForCreateChild) {
  MockZooWatcher watcher;

  EXPECT_CALL(watcher, OnConnected());
  EXPECT_CALL(watcher, OnChildChanged(StrEq("/test"))).Times(1);

  ZooKeeper zk("localhost:2181", &watcher);
  WaitForConnected(zk);

  zk.Create("/test");

  zk.GetChildren("/test", true);
  zk.Create("/test/abc");

  zk.Delete("/test/abc");

  zk.Delete("/test");
  sleep(1);
}

TEST(ZooKeeperWatch, GetChildrenAndWatchForDeleteChild) {
  MockZooWatcher watcher;

  EXPECT_CALL(watcher, OnConnected());
  EXPECT_CALL(watcher, OnChildChanged(StrEq("/test"))).Times(1);

  ZooKeeper zk("localhost:2181", &watcher);
  WaitForConnected(zk);

  zk.Create("/test");

  zk.Create("/test/abc");

  zk.GetChildren("/test", true);
  zk.Delete("/test/abc");

  zk.Delete("/test");
  sleep(1);
}

TEST(ZooKeeperWatch, GetChildrenAndWatchForDeleteParent) {
  MockZooWatcher watcher;

  EXPECT_CALL(watcher, OnConnected());
  EXPECT_CALL(watcher, OnDeleted(StrEq("/test")));

  ZooKeeper zk("localhost:2181", &watcher);
  WaitForConnected(zk);

  zk.Create("/test");

  zk.GetChildren("/test", true);

  zk.Delete("/test");
  sleep(1);
}

TEST(ZooKeeperWatch, GetChildrenAndCantWatchForCreateParent) {
  MockZooWatcher watcher;

  EXPECT_CALL(watcher, OnConnected());

  ZooKeeper zk("localhost:2181", &watcher);
  WaitForConnected(zk);

  EXPECT_THROW(zk.GetChildren("/test", true), ZooException);

  zk.Create("/test");
  zk.Create("/test/abc");
  zk.Delete("/test/abc");

  zk.Delete("/test");
  sleep(1);
}

