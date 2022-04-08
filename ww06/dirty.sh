#!/bin/bash

for ((;;));
do
	cat /proc/meminfo | grep Dirty
	sleep 1
done

