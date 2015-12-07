cmake_minimum_required(VERSION 3.2)

project(zookeeper-cpp)

enable_testing()

set(EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)

set(CMAKE_CXX_STANDARD "14")

# googletest & googlemock
set(GOOGLETEST_ROOT ${EXTERNAL_DIR}/googletest)
add_subdirectory(${GOOGLETEST_ROOT}/googlemock)
set(GTEST_INCLUDE_DIRS "${GOOGLETEST_ROOT}/googletest/include")
set(GMOCK_INCLUDE_DIRS "${GOOGLETEST_ROOT}/googlemock/include")

# executors library
set(EXECUTORS_INCLUDE_DIRS "${EXTERNAL_DIR}/executors/include")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(zookeeper-cpp)

#set(GLOG_ROOT ${EXTERNAL_DIR}/glog)
#add_subdirectory(${GLOG_ROOT})
#set(GLOG_INCLUDE_DIRS ${GLOG_ROOT}/src)
#set(GLOG_INCLUDE_LIBRARIES glog)

if(BUILD_APP)
  add_subdirectory(app/leader-guard)
endif()
