#!/bin/bash

# check if directory 'build' exists, if true: proceeds, else: create directory
if [ -d build ]; then :  
else mkdir build 
fi 

{
# build project
cd build/ && cmake .. && make && cd .. && echo ""\

#get directories
dir=$1
current_dir="$( cd "$( dirname "$0" )" && cd .. && cd .. && pwd )"

#get perforation rates
rates_path="${current_dir}/settings/perforation_rates.txt"
rates=$(<${rates_path})

#get all files in dir 
files=$(find $dir -type f \( -iname \*.c -o -iname \*.cpp \))
#run through files in dir
for src in $files; do
    src_dir=$(dirname $src)
    src_base=$(basename $src)

    dir_path="${src_dir}/memory_skipping_results/"
    mkdir -p $dir_path
    filename=${dir_path}/${src_base%.*}.ll
    opt=${dir_path}/${src_base%.*}.opt.ll
    #filename_perf=${dir_path}/${src_base}_${i}.ll
    clang-12 -x c++ -S -emit-llvm ${src} -g3 -O0 -Xclang -disable-O0-optnone -o ${filename}
    opt-12 -S -mem2reg -loop-simplify ${filename} > ${opt}

    for i in $rates; do
        dir_path="${src_dir}/memory_skipping_results/$i"
        mkdir -p $dir_path
        opt_perf=${dir_path}/${src_base%.*}_${i}.opt.ll
        opt-12 -S -load build/transformations/memory-skipping/libMemorySkippingPass.so -memory-skipping -loop_rate=$i < ${opt} > ${opt_perf}
        echo "Memory Skipping done! You can find results in ${dir_path}"      
    done
done 
} 2> memory-skipping.err