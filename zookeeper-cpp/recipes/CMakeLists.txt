include_directories(${EXECUTORS_INCLUDE_DIRS} ${GLOG_INCLUDE_DIRS})

set(RECIPES_SRCS
    leader_elector.h leader_elector.cpp)

add_library(zookeeper-recipes ${RECIPES_SRCS})

target_compile_features(zookeeper-recipes PRIVATE cxx_return_type_deduction)
target_link_libraries(zookeeper-recipes zookeeper-cpp)

# unit test
include_directories(${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS})
add_executable(recipes_unittest
               leader_elector_unittest.cpp)

target_compile_features(recipes_unittest PRIVATE cxx_return_type_deduction)
target_link_libraries(recipes_unittest
    zookeeper-cpp zookeeper-recipes
    gtest gtest_main gmock)


