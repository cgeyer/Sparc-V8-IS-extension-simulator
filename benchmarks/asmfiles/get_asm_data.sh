#!/bin/sh
#
# SPARC V8 Instruction Set Extension Simulator
#                                                                               
# File: examples/asmfiles/get_asm_data.sh
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
# Extracts several information from the given assembler file.
# Usage: ./get_asm_data.sh <asmfile>
#

echo "Number of conditional branches:";
egrep -c ".*b(e|(ne)|g|(le)|(ge)|l|(gu)|(leu)|(cc)|(cs)|(pos)|(neg)|(vc)|(vs)) \..*" $1;

echo "Number of unconditional branches:";
egrep -c ".*ba \..*" $1;

echo "Number of nops:";
egrep -c ".*nop.*" $1;

echo "Number of MBBs:";
egrep "(^\.LBB.*)|(^\! BB.*)" $1 | tail -n 1

echo "Number of movCCs:";
egrep -c "mov" $1;

echo "Number of selCCs:";
egrep -c "sel" $1;

echo "Number of predicated blocks:";
egrep -c "predbegin" $1;

echo "Number of HWLoops:"
egrep -c "hwloop start" $1;
