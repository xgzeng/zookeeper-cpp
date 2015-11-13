#pragma once

const std::string ZOOKEEPER_DAEMON_PATH = "/Volumes/home/zengxg/projects/zookeeper-3.4.6/bin/zkServer.sh";
const std::string ZOOKEEPER_HOSTS = "127.0.0.1:2181";

inline void StartZookeeper() {
  auto cmd = ZOOKEEPER_DAEMON_PATH + " start";
  system(cmd.c_str());
}

inline void StopZookeeper() {
  auto cmd = ZOOKEEPER_DAEMON_PATH + " stop";
  system(cmd.c_str());
}

inline void WaitForConnected(zookeeper::ZooKeeper& zk) {
  while (!zk.is_connected()) {
    sleep(1);
  }
}

struct ZooKeeperTest : ::testing::Test {
  zookeeper::ZooKeeper zk;

  ZooKeeperTest()
  : zk("localhost:2181") {
    WaitForConnected(zk);
  }
};

