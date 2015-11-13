#include "zookeeper.hpp"

namespace zookeeper {

std::string RecursiveCreate(ZooKeeper& zk,
                            const std::string& path,
                            const std::string& value,
                            int flag) {
  std::string::size_type pos = 0;
  do {
    pos = path.find('/', pos + 1);
    if (pos == std::string::npos) {
      break;
    }

    zk.CreateIfNotExists(path.substr(0, pos));
    pos = pos + 1;
  } while (true);

  return zk.CreateIfNotExists(path, value, flag);
}

} // namespace zookeeper

