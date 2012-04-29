#!/bin/sh

echo "Number of bytes:";
head -n 1 $1 | sed "s/\(Contents of instruction memory (\)\([0-9]*\)\( bytes):.*\)/\2/g";

echo "Max cycles:";
egrep "Current" $1 | sed s/"Current simulated cycles: "//g | awk 'max=="" || $1 > max {max=$1} END{ print max}' FS=".";

echo "Min cycles:";
egrep "Current" $1 | sed s/"Current simulated cycles: "//g | awk 'min=="" || $1 < min {min=$1} END{ print min}' FS=".";

tail -n 2 $1;

exit 0;
