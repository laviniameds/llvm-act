#!/bin/bash

#check if directory 'build' exists, if true: proceeds, else: create directory
if [ -d build ]; then :  
else mkdir build 
fi 

# build project
cd build/ && cmake .. && make && cd .. && echo ""\

#define variables
src=$1
src_name=${1%.c}
standard=${1%/*.c}/results/standard.txt
perforrated=${1%/*.c}/results/perforrated.txt
limit_path=${1%/*.c}/config/limit.txt
perf=${1%/*.c}/perf_options/perf.c
obj=${1%.c}.ll
opt=${1%.c}.opt.ll

#run the example and generate the 'standard' result
clang -Wall $src -o $src_name
./$src_name < $limit_path > $standard
limit=$(<${limit_path})

#run the perforation pass and get the modified example

# run pass 
clang-10 -S -emit-llvm ${src} -g3 -O0 -Xclang -disable-O0-optnone -o ${obj}

opt-10 -S -mem2reg ${obj} > ${opt}

#generate perforated code variants
for (( i=1; i<=$limit; i+=1 ))
do
   perf_option=${perf%.c}_${i}.opt.ll
   opt-10 -S -load build/loop-perforation/libLoopPerforationPass.so -loop-perforation -loop_rate=$i < ${opt} > ${perf_option}
done

#TODO
#run modified example and generates the 'perforrated' result

#TODO 
#run the error metrics to get final results

#opt-10 ${opt} -loops -analyze