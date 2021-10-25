#!/bin/bash
{
#get directory
current_dir="$( cd "$( dirname "$0" )" && pwd )"
#get perforation rates
rates_path="${current_dir}/settings/perforation_rates.txt"
rates=$(<${rates_path})
FLAGS=$2

#get all files in dir 
directories=$(find $1 -type d -name "*_results" )

#run through files in dir
for d in $directories; do
    base_name=$(basename ${d%/*/*})
    #echo $base_name
    for i in $rates; do
        dir_files=${d}/${i}
        #echo ${dir_files}/bin/${base_name}_${i}.o
        mkdir -p ${dir_files}/bin
        files=$(find $dir_files -type f -name "*.opt.ll" )  
        #echo $files
        clang++-12 ${FLAGS} ${dir_files}/*.opt.ll -o ${dir_files}/bin/${base_name}_${i}.out
    done
done 
} 2> run.err