#pragma once
#include <zookeeper/zookeeper.h>
#include <string>

//struct zhandle_t;

namespace zookeeper {

class ZooWatcher {
public:
  virtual ~ZooWatcher() {}

  virtual void OnConnected() = 0;
  virtual void OnConnecting() = 0;
  virtual void OnSessionExpired() = 0;

  virtual void OnCreated(const char* path) = 0;
  virtual void OnDeleted(const char* path) = 0;
  virtual void OnChanged(const char* path) = 0;
  virtual void OnChildChanged(const char* path) = 0;
  virtual void OnNotWatching(const char* path) = 0;

};

class ZooKeeper {
public:
  ZooKeeper(const std::string& server_hosts,
            ZooWatcher* global_watcher = nullptr);

  ~ZooKeeper();

  bool is_connected();

  bool Exists(const char* path);

private:
  zhandle_t* zoo_handle_ = nullptr;

  ZooWatcher* global_watcher_ = nullptr;

  void WatchHandler(int type, int state, const char* path);

  static void GlobalWatchFunc(zhandle_t*, int type, int state,
                              const char* path, void* ctx);
};

}

