#!/bin/bash
find twitter -type f -name "*.edges" | while read file;
do
    f=$(basename "$file")
    f1=${f%.*}
    mkdir  "$file"_
#    mv "$f" "$f1"
done  
