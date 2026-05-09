cmake_minimum_required(VERSION 3.10)
project(unmacro)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Clang REQUIRED CONFIG)

message(STATUS "Found Clang ${Clang_VERSION}")
include_directories(${CLANG_INCLUDE_DIRS})

set(SOURCES
  src/unmacro.cpp
  src/UnmacroPPCallbacks.cpp
  src/BodyVisitor.cpp
  src/UnmacroAction.cpp
)

add_executable(unmacro ${SOURCES})

target_link_libraries(unmacro PRIVATE
  clang-cpp
  LLVM
)
