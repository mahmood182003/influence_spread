#!/bin/bash

for d in data/twitter/*/seeds* ;
do
	seedsX="$(basename  "$d")"
	echo $d
	(cd "$d" && convert '*.png' -set filename:num '%[fx:(n-1)]' "$seedsX"_spread%[filename:num]_ratio%%_greedy1.pdf; rm *.png *.dot);	
done
