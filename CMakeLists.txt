cmake_minimum_required(VERSION 3.10)
project(unmacro)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Clang REQUIRED CONFIG)

message(STATUS "Found Clang ${Clang_VERSION}")
include_directories(${CLANG_INCLUDE_DIRS})

add_executable(unmacro src/unmacro.cpp)

target_link_libraries(unmacro PRIVATE
  clang-cpp
  LLVM
)
