cmake_minimum_required(VERSION 3.2)

project(zookeeper-cpp)

enable_testing()

#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/googletest/googletest)
set(GOOGLETEST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/external/googletest")
add_subdirectory(${GOOGLETEST_ROOT}/googlemock)
set(GTEST_INCLUDE_DIRS "${GOOGLETEST_ROOT}/googletest/include")
set(GMOCK_INCLUDE_DIRS "${GOOGLETEST_ROOT}/googlemock/include")

#
#find_package(Boost COMPONENTS system)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(zookeeper-cpp)

add_subdirectory(app/leader-guard)

