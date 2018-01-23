#!/bin/bash

for d in data/twitter/*/seeds* ;
do
	seedsX="$(basename  "$d")"
	echo $d
	(cd "$d"; rm *.png *.dot);	
done
