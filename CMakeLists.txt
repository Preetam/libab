cmake_minimum_required(VERSION 2.8)

project(sacq)
SET(CMAKE_CXX_FLAGS "-std=c++14")
include_directories(
	include
	src
	dep/libuv/include
)

add_subdirectory(dep/glog)
add_subdirectory(dep/libuv)

add_executable(sacq
	src/message/message.cc
	src/peer/peer.cc
	src/node/node.cc
	src/node/role.cc
	src/main.cc
)

target_link_libraries(sacq glog)
target_link_libraries(sacq uv)
