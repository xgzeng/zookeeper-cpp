
set(ZOOKEEPER_SRCS
    zookeeper.hpp zookeeper.cpp
    zookeeper_error.hpp zookeeper_error.cpp
    zookeeper_ext.hpp zookeeper_ext.cpp
    )

add_library(zookeeper-cpp ${ZOOKEEPER_SRCS})
target_link_libraries(zookeeper-cpp zookeeper_mt)

# Tests
include_directories(${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS})

set(ZOOKEEPER_UNITTEST_SRCS
    zookeeper_mock.hpp
    zookeeper_unittest_helper.hpp
    zookeeper_unittest.cpp
    zookeeper_ext_unittest.cpp
    )

add_executable(zookeeper_unittest ${ZOOKEEPER_UNITTEST_SRCS})

target_link_libraries(zookeeper_unittest
    zookeeper-cpp gtest gtest_main gmock)

add_subdirectory(recipes)
