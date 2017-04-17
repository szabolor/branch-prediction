#!/bin/sh

# Testing with 0s and 1s distinct group sets (integer partitions)
# Maybe it's not the best for the brach predictor, because 0...00000110111011 can cause
# different result than 0...00000111011011 (rearranged group order), but the amount of the
# bits should do the approximately same job.

trap 'echo "exiting..."; exit;' INT

SERIES_FILE="A194602.list"
BIT_LENGTH=$1
LIMIT=$(echo "2^($BIT_LENGTH-1)" | bc)
REPEAT=2

for i in $(cat $SERIES_FILE);
do
	if [ "$i" -gt "$LIMIT" ]; then
	    break
	fi
	echo "Trying $i ($BIT_LENGTH bit)..."
	echo $BIT_LENGTH";" $i";" $(perf stat -r $REPEAT -x ";" -e branch-misses ./branch_test $BIT_LENGTH $i 2>&1 | cut -d";" -f1,4) >> shr_result
done
