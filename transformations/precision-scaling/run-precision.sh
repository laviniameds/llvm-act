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

#get all files in dir 
files=$(find $dir -type f \( -iname \*.c -o -iname \*.cpp \))
#run through files in dir
for src in $files; do
    src_dir=$(dirname $src)
    src_base=$(basename $src)
    dir_path="${src_dir}/precision_scaling_results"
    mkdir -p $dir_path
    filename=${dir_path}/${src_base%.*}.ll
    opt=${dir_path}/${src_base%.*}.opt.ll
    clang-12 -x c++ -S -emit-llvm ${src} -g3 -O0 -Xclang -disable-O0-optnone -o ${filename}
    opt-12 -S -mem2reg -loop-simplify ${filename} > ${opt}

    # if [ "${base_src_dir}" == "${base_filename%.*}" ]
    # then
        #echo ${base_src_dir} ${base_filename%.*}
        #for i in $rates; do
            dir_path="${src_dir}/precision_scaling_results/"
            mkdir -p $dir_path
            opt_perf=${dir_path}/${src_base%.*}_modified.opt.ll    
            #echo $opt_perf       
            opt-12 -S -load build/transformations/precision-scaling/libPrecisionScalingPass.so -precision-scaling < ${opt} > ${opt_perf}      
            #opt-12 -analyze -dot-cfg ${opt}
            echo "Precision Scaling done! You can find results in ${dir_path}"        
        #done
    # fi
done 
} 2> precision-scaling.err