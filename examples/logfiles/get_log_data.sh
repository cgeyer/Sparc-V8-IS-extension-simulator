#!/bin/sh
#
# SPARC V8 Instruction Set Extension Simulator
#                                                                               
# File: examples/logfiles/get_log_data.sh
#                                                                               
# Copyright (c) 2012 Clemens Bernhard Geyer <clemens.geyer@gmail.com>
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy 
# of this software and associated documentation files (the "Software"), to 
# deal in the Software without restriction, including without limitation the 
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#                                                                               
# The above copyright notice and this permission notice shall be included in 
# all copies or substantial portions of the Software.
#                                                                               
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
# THE SOFTWARE.
#
# Extracts several information from the given simulator log file.
# Usage: ./get_log_data.sh <logfile>
#


echo "Number of bytes:";
head -n 1 $1 | sed "s/\(Contents of instruction memory (\)\([0-9]*\)\( bytes):.*\)/\2/g";

echo "Max cycles:";
egrep "Current" $1 | sed s/"Current simulated cycles: "//g | awk 'max=="" || $1 > max {max=$1} END{ print max}' FS=".";

echo "Min cycles:";
egrep "Current" $1 | sed s/"Current simulated cycles: "//g | awk 'min=="" || $1 < min {min=$1} END{ print min}' FS=".";

tail -n 2 $1;

exit 0;
