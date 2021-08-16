#!/bin/bash
find . -name "*.log" -type f -delete
find . -name "*.err" -type f -delete

dir=$1
#get all files in dir 
files=$(find $dir -type f \( -iname \*.ll -o -iname \*.txt -o -iname \*.err -o -iname \*.log \))

for src in $files; do
    rm $src
done