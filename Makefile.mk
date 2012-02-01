# SPARC V8 Instruction Set Extension Simulator
#
# File: Makefile.mk
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
 
YACCFILE=sparc.y
FLEXFILE=sparc.l
YACCCFILE=$(YACCFILE:.y=.tab.c)
FLEXCFILE=$(FLEXFILE:.l=.flex.c)
YYCFILES=$(YACCCFILE) $(FLEXCFILE)
YYINCLUDEFILE=$(YACCFILE:.y=.tab.h)
YYPREFIX=$(basename $(YACCFILE))

ASMCFILES=asm_main.c gen_asm.c
SIMCFILES=sim_main.c gen_sim.c

SHCFILES=libasm_sparc_v8.c libsim_sparc_v8.c \
libasm_sparc_v8-blockicc-movcc.c libsim_sparc_v8-blockicc-movcc.c \
libasm_sparc_v8-blockpreg-selcc.c libsim_sparc_v8-blockpreg-selcc.c \
libasm_sparc_v8-blockicc-selcc.c libsim_sparc_v8-blockicc-selcc.c 
SHARED_OBJS=$(SHCFILES:.c=.so)
