#!/bin/bash
#run with sudo

cd build

#for includes first and last number
for i in {0..500}
do
	printf "\n\n/////////////////////////////////////Run Number $i////////////////////////////////////////\n\n\n\n"
	./CamSim $i
	 xdotool click 1
done

cd ..


