cmake_minimum_required(VERSION 3.1)
project(llvm-act)

# support C++14 features used by LLVM 10.0.0
set(CMAKE_CXX_STANDARD 14)

find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

# Use your pass name here.
add_subdirectory(analysis/profiler)
add_subdirectory(transformations/loop-perforation)
add_subdirectory(transformations/memory-skipping)
add_subdirectory(transformations/precision-scaling)
#add_subdirectory(transformations/float2int)