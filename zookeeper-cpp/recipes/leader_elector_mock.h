#pragma once
#include "leader_elector.h"
#include <gmock/gmock.h>

namespace zookeeper {

class MockLeaderElectorHandler : public LeaderElectorHandler {
public:
  MOCK_METHOD0(TakeLeadership, void());
  MOCK_METHOD0(RevokeLeadership, void());
  MOCK_METHOD1(LeadershipChanged, void(const std::string& current_leader));
};

}

