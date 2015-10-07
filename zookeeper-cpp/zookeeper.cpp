#include "zookeeper.hpp"
#include <cassert>
#include <cerrno>
#include <mutex>
#include "zookeeper_error.hpp"

namespace zookeeper {

void ZooKeeper::GlobalWatchFunc(zhandle_t* h, int type, int state, const char* path, void* ctx) {
  auto self = static_cast<ZooKeeper*>(ctx);
  self->WatchHandler(type, state, path);
}

static std::once_flag ONCE_FLAG_SET_DEBUG_LEVEL;
void set_default_debug_level() {
  std::call_once(ONCE_FLAG_SET_DEBUG_LEVEL, []{
    zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
  });
}

ZooKeeper::ZooKeeper(const std::string& server_hosts,
                     ZooWatcher* global_watcher)
: global_watcher_(global_watcher) {
  set_default_debug_level();

  zoo_handle_ = zookeeper_init(server_hosts.c_str(), GlobalWatchFunc, 2 * 1000,
                               nullptr, this, 0);
  if (!zoo_handle_) {
    throw ZooSystemErrorFromErrno(errno);
  }
}

ZooKeeper::~ZooKeeper() {
  if (zoo_handle_) {
    auto ret = zookeeper_close(zoo_handle_);
    if (ret != ZOK) {
      // TODO: log information
    }
  }
}

bool ZooKeeper::is_connected() {
  return zoo_state(zoo_handle_) == ZOO_CONNECTED_STATE;
}

void ZooKeeper::WatchHandler(int type, int state, const char* path) {
  // call global watcher
  if (!global_watcher_) return;

  if (type == ZOO_SESSION_EVENT) {
    if (state == ZOO_EXPIRED_SESSION_STATE) {
      global_watcher_->OnSessionExpired();
    } else if (state == ZOO_CONNECTED_STATE) {
      global_watcher_->OnConnected();
    } else if (state == ZOO_CONNECTING_STATE) {
      global_watcher_->OnConnecting();
    } else {
      // TODO:
      assert(0 && "don't know how to process other session event yet");
    }
  } else if (type == ZOO_CREATED_EVENT) {
    global_watcher_->OnCreated(path);
  } else if (type == ZOO_DELETED_EVENT) {
    global_watcher_->OnDeleted(path);
  } else if (type == ZOO_CHANGED_EVENT) {
    global_watcher_->OnChanged(path);
  } else if (type == ZOO_CHILD_EVENT) {
    global_watcher_->OnChildChanged(path);
  } else if (type == ZOO_NOTWATCHING_EVENT) {
    global_watcher_->OnNotWatching(path);
  } else {
    assert(false && "unknown zookeeper event type");
  }
}

bool ZooKeeper::Exists(const char* path) {
  auto zoo_code = zoo_exists(zoo_handle_, path, false, nullptr);
  switch (zoo_code) {
  case ZOK:
    return true;
  case ZNONODE:
    return false;
  default:
    throw ZooException(zoo_code);
  }
}

}

