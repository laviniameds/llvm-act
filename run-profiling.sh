#!/bin/bash

# check if directory 'build' exists, if true: proceeds, else: create directory
if [ -d build ]; then :  
else mkdir build 
fi 

# build project
cd build/ && cmake .. && make && cd .. && echo ""\

# define variables
src=$1
#TODO: .c/.cpp
obj=${1%.cpp}.ll

clang-12 -S -emit-llvm ${src} -g3 -O0 -Xclang -disable-O0-optnone -o ${obj}

opt-12 -disable-output -load-pass-plugin=build/analysis/profiling/libProfiling.so -passes="profiling" ${obj} 