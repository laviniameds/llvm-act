#!/bin/bash

#check if directory 'build' exists, if true: proceeds, else: create directory
if [ -d build ]; then :  
else mkdir build 
fi 

# build project
cd build/ && cmake .. && make && cd .. && echo ""\

src=$1
src_name=${1%.c}
standard=${1%/*.c}/results/standard
perforrated=${1%/*.c}/results/perforrated

obj=${1%.c}.ll
opt=${1%.c}.opt.ll

#run the example and generate the 'standard' result
clang-10 -Wall $src -o $src_name
./$src_name > $standard

#run the perforation pass and get the modified example

# # pass testing 
# clang-10 -S -emit-llvm ${src} -g3 -O0 -Xclang -disable-O0-optnone -o ${obj}

# opt-10 -S -mem2reg ${obj} > ${opt}

# #opt -analyze -dot-cfg ${opt}

# opt-10 -load build/loop-perforation/libLoopPerforationPass.so -loop-perforation < ${opt} > /dev/null

#TODO
#run modified example and generates the 'perforrated' result

#TODO 
#run the error metrics to get final results