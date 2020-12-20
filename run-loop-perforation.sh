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
obj=${1%.c}.ll
opt=${1%.c}.opt.ll

#run the example and generate the 'standard' result
clang -Wall $src -o $src_name
./$src_name < $limit_path > $standard

#run the perforation pass and get the modified example

# run pass 
clang-10 -S -emit-llvm ${src} -g3 -O0 -Xclang -disable-O0-optnone -o ${obj}

opt-10 -S -mem2reg ${obj} > ${opt}

opt-10 -load build/loop-perforation/libLoopPerforationPass.so -loop-perforation < ${opt} > /dev/null

#opt-10 ${opt} -loops -analyze

#TODO
#run modified example and generates the 'perforrated' result

#TODO 
#run the error metrics to get final results