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
perforated=${1%/*.c}/results/perforated.txt
rates_path=${1%/*.c}/config/rates.txt
perf=${1%/*.c}/perf_options/perf.c
merged_results=${1%/*.c}/perf_options/merged_results.txt
obj=${1%.c}.ll
opt=${1%.c}.opt.ll
error_metrics=${1%/*.c}/error_metrics/error_metrics.c
error_metrics_name=${error_metrics%.c}
final_results=${1%/*.c}/results/final_results.txt

#run the example and generate the 'standard' result
clang-10 -Wall $src -o $src_name
./$src_name > $standard
standard_result=$(<${standard})

#get rates from file
rates=$(<${rates_path})

#init final results file
if [ -e $final_results ]; then 
    > $final_results
    echo -e "--- RESULTS: ---\n" >> $final_results
else 
    echo -e "--- RESULTS: ---\n" >> $final_results 
fi 

# run pass 
clang-10 -S -emit-llvm ${src} -g3 -O0 -Xclang -disable-O0-optnone -o ${obj}

opt-10 -S -mem2reg ${obj} > ${opt}

#test perforation
for i in $rates
do
    perf_option=${perf%.c}_${i}.opt.ll
    perf_option_name=${perf%.c}_${i}
    #perforated_option=${perforated%.txt}_$i.txt

    #generate perforated code variants
    opt-10 -S -load build/loop-perforation/libLoopPerforationPass.so -loop-perforation -loop_rate=$i < ${opt} > ${perf_option}
    clang-10 -Wall $perf_option -o $perf_option_name

    #get perforated result
    perf_result=$(./$perf_option_name)
    #merge standard and perforated results
    echo "$standard_result $perf_result" > $merged_results

    #test error metrics function
    clang-10 -Wall ${error_metrics} -o ${error_metrics_name}
    error_ratio=$(./$error_metrics_name < $merged_results)

    echo -e "LOOP RATE: $i" >> $final_results
    echo -e "STANDARD RESULT: $standard_result --- PERFORATED RESULT: $perf_result" >> $final_results
    echo -e "ERROR RATIO: $error_ratio\n\n" >> $final_results
done

#opt-10 ${opt} -loops -analyze