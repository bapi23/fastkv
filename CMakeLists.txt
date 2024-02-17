cmake_minimum_required(VERSION 2.81)

project(fastkv C CXX)
find_package (Seastar REQUIRED)
find_package(Catch2 3 REQUIRED)

add_executable (fastkv 
src/main.cpp 
src/cache/lru_cache.cpp
src/storage/storage.cpp

src/handlers/delete.cpp
src/handlers/get_keys.cpp
src/handlers/get_val.cpp
src/handlers/store_val.cpp
src/handlers/routes.cpp
)
set_property(TARGET fastkv PROPERTY CXX_STANDARD 20)
target_include_directories(fastkv PRIVATE include)
target_link_libraries (fastkv Seastar::seastar)


add_executable(test 
tests/unit/main_test.cpp 
src/cache/lru_cache.cpp
)

add_test(
  NAME test
  COMMAND $<TARGET_FILE:cpp_test>
  )

target_link_libraries(test Catch2::Catch2WithMain Seastar::seastar)
target_include_directories(test PRIVATE include)
set_property(TARGET test PROPERTY CXX_STANDARD 20)
  


