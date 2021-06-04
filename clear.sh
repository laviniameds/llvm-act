#!/bin/bash
find . -name "*.log" -type f -delete
find . -name "*.err" -type f -delete

dir=$1
#get all files in dir 
files=$(find $dir -type f \( -iname \*.ll \))

for src in $files; do
    rm $src
done