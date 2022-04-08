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
    opt-12 -S -load build/analysis/profiling/libProfiling.so -profiling-pass < ${opt} > /dev/null
done

#get all files in dir 
directories=$(find $1 -type d -name "*profiling_results" )

#run through files in dir
for d in $directories; do
    dir=$(dirname ${d%.*})
    d_base=$(basename $dir)

    mkdir -p ${d}/results
    echo "Binary,Control Flow,Memory," > ${d}/results/merged.csv
    echo "Binary,Control Flow,Memory," > ${d}/results/sum.csv

    for fname in ${d}/*.csv; do
        tail -n +2 -q $fname >> ${d}/results/merged.csv
    done

    awk ' BEGIN {FS=OFS=","}
    {for (i=1; i<NF; i++) {sum[i]+=$i} len=NF}
    END {for (i=1; i<=len; i++) $i=sum[i]; print}
    ' ${d}/results/merged.csv >> ${d}/results/sum.csv
done

} 2> profiling.err