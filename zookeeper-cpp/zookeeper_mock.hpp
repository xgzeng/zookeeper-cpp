#pragma once
#include <gmock/gmock.h>
#include "zookeeper.hpp"

namespace zookeeper {

class MockZooWatcher : public ZooWatcher {
public:
  MOCK_METHOD0(OnConnected, void());
  MOCK_METHOD0(OnConnecting, void());
  MOCK_METHOD0(OnSessionExpired, void());

  MOCK_METHOD1(OnCreated, void(const char*));
  MOCK_METHOD1(OnDeleted, void(const char*));
  MOCK_METHOD1(OnChanged, void(const char*));
  MOCK_METHOD1(OnChildChanged, void(const char*));
  MOCK_METHOD1(OnNotWatching, void(const char*));
};

}
