cmake_minimum_required(VERSION 2.81)

project(scylladb_assignment C CXX)
add_executable (scylladb_assignment src/main.cpp)

set_property(TARGET scylladb_assignment PROPERTY CXX_STANDARD 20)
find_package (Seastar REQUIRED)
  
target_link_libraries (scylladb_assignment
  Seastar::seastar)