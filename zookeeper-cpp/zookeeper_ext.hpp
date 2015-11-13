#pragma once
#include <string>

namespace zookeeper {

class ZooKeeper;

std::string RecursiveCreate(ZooKeeper& zk,
                            const std::string& path,
                            const std::string& value = std::string(),
                            int flag = 0);

}


