#!/bin/bash

# check if directory 'build' exists, if true: proceeds, else: create directory
if [ -d build ]; then :  
else mkdir build 
fi 

# build project
cd build/ && cmake .. && make && cd .. && echo ""\

#get directory
dir=$1

#name_dir=$(basename ${dir})
#mkdir -p analysis/profiling/results/"$name_dir"

#get all files in dir 
files=$(find $dir -type f \( -iname \*.c -o -iname \*.cpp \))
#run through files in dir
for src in $files; do
    filename=${src%.cpp}.ll
    opt=${src%.cpp}.opt.ll
    clang-12 -x c++ -S -emit-llvm ${src} -g3 -O0 -Xclang -disable-O0-optnone -o ${filename}
    opt-12 -S -mem2reg ${filename} > ${opt}
    opt-12 -S -load build/transformations/memory-skipping/libMemorySkippingPass.so -memory-skipping < ${opt} > /dev/null   
done