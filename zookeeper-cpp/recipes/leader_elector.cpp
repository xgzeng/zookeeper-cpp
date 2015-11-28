#include "leader_elector.h"
#include <zookeeper-cpp/zookeeper_ext.hpp>
#include <thread>
#include <chrono>
#include <algorithm>

using std::experimental::post;
using namespace zookeeper;

LeaderElector::LeaderElector(const std::string& zookeeper_servers,
                             const std::string& election_path,
                             LeaderElectorHandler * handler)
: zookeeper_servers_(zookeeper_servers),
  election_path_(election_path),
  leadership_handler_(handler),
  executor_(std::experimental::system_executor()) {
  assert(leadership_handler_);
  zk_ = std::make_unique<ZooKeeper>(zookeeper_servers_, this);
}

void LeaderElector::ResetZooKeeperClient() {
  zk_ = std::make_unique<ZooKeeper>(zookeeper_servers_, this);
}

LeaderElector::~LeaderElector() {
}

void LeaderElector::is_leader(bool value) {
  is_leader_ = value;
  if (is_leader_) {
    assert(!election_sequence_node_.empty());
  }
}

void LeaderElector::Join() {
  is_elector_ = true;
  RefreshLater();
}

void LeaderElector::Leave() {
  is_elector_ = false;
  RefreshLater();
}

void LeaderElector::RefreshLater() {
  post(executor_, [this](){ this->Refresh(); });
}

void LeaderElector::Refresh() {
  printf("Refresh\n");
  if (zk_->is_connected()) {
    // new session or reestablished connection
    if (is_elector_) {
      try {
        EnterElection();
      } catch(...) {
        printf("EnterElection failed\n");
      }
    } else {
      ExitElection();
    }
  } else if (zk_->is_expired()) {
    // session expired
    if (is_leader()) {
      RevokeLeadershipImpl();
    }
    election_sequence_node_.clear();
    ResetZooKeeperClient();
  } else {
    // disconnected from zookeeper
    if (is_leader()) {
      RevokeLeadershipImpl();
    }
  }
}

void LeaderElector::EnterElection() {
  printf("EnterElection\n");
  // create election directory
  RecursiveCreate(*zk_, election_path_);

  if (!election_sequence_node_.empty()
      && !zk_->Exists(election_sequence_node_)) {
    // TODO: LOG election node deleted unexpectedlly
    election_sequence_node_.clear();
  }

  if (election_sequence_node_.empty()) {
    // TODO: what if sequence node is create, and node name isn't returned
    election_sequence_node_ = zk_->Create(election_path_ + "/proc_",
                                          "", ZOO_SEQUENCE | ZOO_EPHEMERAL);
    printf("I'm %s\n", election_sequence_node_.c_str());
  }

  // watch for election node
  post(executor_, [this](){
    this->OnElectionChanged();
  });
}

void LeaderElector::ExitElection() {
  assert(!is_elector_);

  if (election_sequence_node_.empty()) {
    assert(!is_leader());
    return;
  }

  try {
    zk_->DeleteIfExists(election_sequence_node_);
  } catch (std::exception &e) {
    printf("can't exit election gracefully, %s", e.what());
    ResetZooKeeperClient();
  }

  election_sequence_node_.clear();

  is_leader(false);
}

void LeaderElector::OnElectionChanged() {
  printf("OnElectionChanged\n");
  if (!is_elector_ || election_sequence_node_.empty()) {
    return;
  }

  // fetch all election nodes and watch for changes
  auto procs = zk_->GetChildren(election_path_, true);
  std::sort(std::begin(procs), std::end(procs));
  assert(!procs.empty());

  auto current_leader = election_path_ + '/' + procs[0];

  if (current_leader == election_sequence_node_) {
    // Yelp, I'm Leader Now. :)
    if (!is_leader()) TakeLeadershipImpl();
  } else {
    if (is_leader()) {
      assert(false && "I'm alive but lost leadership");
    } else {
      LeadershipChangedImpl(current_leader);
    }
  }
}

void LeaderElector::TakeLeadershipImpl() {
  assert(!is_leader());
  printf("I'm leader now\n");
  is_leader(true);
  leadership_handler_->TakeLeadership();
}

void LeaderElector::RevokeLeadershipImpl() {
  printf("My leadership is revoked\n");
  is_leader(false);
  if (is_elector_) {
    leadership_handler_->RevokeLeadership();
  }
}

void LeaderElector::LeadershipChangedImpl(const std::string& leader_data) {
  printf("I'm follower, following %s\n", leader_data.c_str());
  leadership_handler_->LeadershipChanged(leader_data);
}

void LeaderElector::OnConnected() {
  printf("OnConnected\n");
  RefreshLater();
}

void LeaderElector::OnConnecting() {
  printf("OnConnecting\n");
  RefreshLater();
}

void LeaderElector::OnSessionExpired() {
  printf("Session Expired\n");
}

void LeaderElector::OnChildChanged(const char* path) {
  if (path == election_path_) {
    OnElectionChanged();
  }
}

void LeaderElector::OnCreated(const char* path) {}
void LeaderElector::OnDeleted(const char* path) {}
void LeaderElector::OnChanged(const char* path) {}
void LeaderElector::OnNotWatching(const char* path) {}

