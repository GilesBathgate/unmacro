cmake_minimum_required(VERSION 3.10)
project(unmacro)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Match LLVM's build configuration to avoid ABI issues
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

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
