#pragma once

#include <zookeeper-cpp/zookeeper.hpp>
#include <experimental/executor>

class LeaderElectorHandler {
public:
  virtual void TakeLeadership() = 0;

  virtual void RevokeLeadership() = 0;

  virtual void LeadershipChanged(const std::string& current_leader) = 0;

protected:
  ~LeaderElectorHandler() {}
};

class LeaderElector : public zookeeper::ZooWatcher {
public:
  LeaderElector(const std::string& zookeeper_servers,
                const std::string& election_path,
                LeaderElectorHandler* handler);

  ~LeaderElector();

  void Join();

  void Leave();

  bool is_leader() const {
    return is_leader_;
  }

private:
  // ZooWatcher callbacks
  void OnConnected() override;
  void OnConnecting() override;
  void OnSessionExpired() override;

  void OnCreated(const char* path) override;
  void OnDeleted(const char* path) override;
  void OnChanged(const char* path) override;
  void OnChildChanged(const char* path) override;
  void OnNotWatching(const char* path) override;

private:
  void is_leader(bool value);

  //
  void Refresh();
  void RefreshLater();

  void EnterElection();
  void ExitElection();

  void OnElectionChanged();

private:
  bool is_leader_ = false;
  void TakeLeadershipImpl();
  void RevokeLeadershipImpl();
  void LeadershipChangedImpl(const std::string& leader_data);

private:
  const std::string zookeeper_servers_;
  const std::string election_path_;

  LeaderElectorHandler * const leadership_handler_;

  std::unique_ptr<zookeeper::ZooKeeper> zk_;
  void ResetZooKeeperClient();

  std::experimental::executor executor_;

  bool is_elector_ = false;

  std::string election_sequence_node_;
};

