#!/bin/bash

#go to default directory
#cd .. && cd ..
# check if directory 'build' exists, if true: proceeds, else: create directory
if [ -d build ]; then :  
else mkdir build 
fi 

{
# build project
cd build/ && cmake .. && make && cd .. && echo ""\ 

#get directory
dir=$1

#get all files in dir 
files=$(find $dir -type f \( -iname \*.c -o -iname \*.cpp \))
#run through files in dir
for src in $files; do
    src_dir=$(dirname $src)
    src_base=$(basename $src)
    dir_path="${src_dir}/profiling_results"
    mkdir -p $dir_path
    filename=${dir_path}/${src_base%.*}.ll
    opt=${dir_path}/${src_base%.*}.opt.ll
    clang-12 -x c++ -S -emit-llvm ${src} -g3 -O0 -Xclang -disable-O0-optnone -o ${filename} 
    opt-12 -S -mem2reg ${filename} > ${opt} 
    opt-12 -S -disable-output -load-pass-plugin=build/analysis/profiling/libProfiling.so -passes="profiling" < ${opt} > /dev/null
    echo "Profilling done! You can find results in $(dirname ${src})"
done
} 2> profiling.err