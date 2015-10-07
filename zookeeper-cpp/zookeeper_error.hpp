#pragma once
#include <stdexcept>

namespace zookeeper {

class ZooException : public std::runtime_error {
public:
  ZooException(int code);
  ZooException(int code, const char* what);

  int code() const {
    return zoo_error_code_;
  }

private:
  int zoo_error_code_;
};

ZooException ZooSystemErrorFromErrno(int);
}
