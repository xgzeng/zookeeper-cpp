
set(ZOOKEEPER_SRCS zookeeper.hpp zookeeper.cpp
    zookeeper_error.hpp zookeeper_error.cpp
    )

include_directories(${GTEST_INCLUDE_DIRS})

add_library(zookeeper-cpp ${ZOOKEEPER_SRCS})
target_link_libraries(zookeeper-cpp zookeeper_mt)

set(ZOOKEEPER_UNITTEST_SRCS zookeeper_unittest)
add_executable(zookeeper_unittest ${ZOOKEEPER_UNITTEST_SRCS})

target_link_libraries(zookeeper_unittest
    zookeeper-cpp ${GTEST_BOTH_LIBRARIES})

