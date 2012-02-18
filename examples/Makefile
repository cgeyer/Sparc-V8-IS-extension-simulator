# SPARC V8 Instruction Set Extension Simulator
#                                                                               
# File: examples/Makefile
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
# Do not forget to set LLVMDIR properly!
#

LLVMDIR=$(HOME)/llvm/Release/bin
LLVMC=$(LLVMDIR)/llvmc
LLC=$(LLVMDIR)/llc

CDIR=cfiles
LLDIR=llvmfiles
ASMDIR=asmfiles
BINDIR=binfiles
LOGDIR=logfiles

SRCFILES=bubble_v1.c bubble_v2.c bubble_v4.c bubble_v5.c bubble_v6.c \
findfirst_v1.c findfirst_v2.c findfirst_v3.c findfirst_v5.c findfirst_v6.c \
bs_v1.c bs_v2.c bs_v3.c bs_v5.c bs_v7.c bs_v8.c bs_v10.c \
inccounter_v1.c inccounter_v2.c inccounter_v4.c

CFILES=$(addprefix $(CDIR)/, $(SRCFILES))
LLFILES=$(SRCFILES:.c=.ll)
V8_SFILES=$(addprefix $(ASMDIR)/, $(SRCFILES:.c=_v8.s))
V8_BINFILES=$(addprefix $(BINDIR)/, $(SRCFILES:.c=_v8.bin))
V8_LOGFILES=$(addprefix $(LOGDIR)/, $(SRCFILES:.c=_v8.log))
SELCC_SFILES=$(addprefix $(ASMDIR)/, $(SRCFILES:.c=_selcc.s))
SELCC_BINFILES=$(addprefix $(BINDIR)/, $(SRCFILES:.c=_selcc.bin))
SELCC_LOGFILES=$(addprefix $(LOGDIR)/, $(SRCFILES:.c=_selcc.log))
MOVCC_SFILES=$(addprefix $(ASMDIR)/, $(SRCFILES:.c=_movcc.s))
MOVCC_BINFILES=$(addprefix $(BINDIR)/, $(SRCFILES:.c=_movcc.bin))
MOVCC_LOGFILES=$(addprefix $(LOGDIR)/, $(SRCFILES:.c=_movcc.log))
PREDBLOCKICC_SFILES=$(addprefix $(ASMDIR)/, $(SRCFILES:.c=_predblockicc.s))
PREDBLOCKICC_BINFILES=$(addprefix $(BINDIR)/, $(SRCFILES:.c=_predblockicc.bin))
PREDBLOCKICC_LOGFILES=$(addprefix $(LOGDIR)/, $(SRCFILES:.c=_predblockicc.log))
HWLOOP_SFILES=$(addprefix $(ASMDIR)/, $(SRCFILES:.c=_hwloop.s))
HWLOOP_BINFILES=$(addprefix $(BINDIR)/, $(SRCFILES:.c=_hwloop.bin))
HWLOOP_LOGFILES=$(addprefix $(LOGDIR)/, $(SRCFILES:.c=_hwloop.log))

TARGET_V8=v8
TARGET_SELCC=v8-blockpreg-selcc
TARGET_MOVCC=v8-blockicc-movcc
TARGET_PREDBLOCKICC=v8-blockicc-selcc
TARGET_HWLOOP=v8-blockpreg-selcc

LLVMC_FLAGS=-S -emit-llvm -O3 -fno-unroll-loops
#LLVMC_FLAGS=-S -emit-llvm -O3
LLC_V8_FLAGS=-march=cbg -mcpu=$(TARGET_V8) -filetype=asm
LLC_SELCC_FLAGS=-march=cbg -mcpu=$(TARGET_SELCC) -mattr=-predblocksreg,-hwloopopt -filetype=asm
#LLC_SELCC_FLAGS=-march=cbg -mcpu=$(TARGET_SELCC) -mattr=-hwloopopt -filetype=asm
LLC_MOVCC_FLAGS=-march=cbg -mcpu=$(TARGET_MOVCC) -mattr=-predblockscc,-hwloopopt -filetype=asm
#LLC_MOVCC_FLAGS=-march=cbg -mcpu=$(TARGET_MOVCC) -mattr=-hwloopopt -filetype=asm
LLC_PREDBLOCKICC_FLAGS=-march=cbg -mcpu=$(TARGET_PREDBLOCKICC) -mattr=-selcc,-singleloop -filetype=asm
LLC_HWLOOP_FLAGS=-march=cbg -mcpu=$(TARGET_HWLOOP) -mattr=-selcc,-predblocksreg,-hwloopopt -filetype=asm

ASM=./assembler
SIM=./simulator


all: $(V8_LOGFILES) $(SELCC_LOGFILES) $(MOVCC_LOGFILES) $(PREDBLOCKICC_LOGFILES) $(HWLOOP_LOGFILES)

fresh: clean all

.PRECIOUS: $(V8_SFILES) $(V8_BINFILES) $(SELCC_SFILES) $(SELCC_BINFILES)\
	$(MOVCC_SFILES) $(MOVCC_BINFILES) $(PREDBLOCKICC_SFILES) $(PREDBLOCKICC_BINFILES) \
	$(HWLOOP_SFILES) $(HWLOOP_BINFILES)

%.ll: $(CDIR)/%.c
	@echo Creating $@...
	@$(LLVMC) $(LLVMC_FLAGS) -o $@ $<


# Rules for Sparc-V8 target
$(ASMDIR)/%_v8.s: %.ll
	@echo Creating $@...
	@$(LLC) $(LLC_V8_FLAGS) -o $@ $<

$(BINDIR)/%_v8.bin: $(ASMDIR)/%_v8.s
	@echo Assembling $<
	@cd ..;\
	$(ASM) -i examples/$< -o examples/$@ -t $(TARGET_V8)

$(LOGDIR)/%_v8.log: $(BINDIR)/%_v8.bin
	@echo Simulating $<
	@cd ..;\
	$(SIM) -i examples/$< -o examples/$@ -t $(TARGET_V8) -s


# Rules for SelCC target
$(ASMDIR)/%_selcc.s: %.ll
	@echo Creating $@...
	@$(LLC) $(LLC_SELCC_FLAGS) -o $@ $<

$(BINDIR)/%_selcc.bin: $(ASMDIR)/%_selcc.s
	@echo Assembling $<
	@cd ..;\
	$(ASM) -i examples/$< -o examples/$@ -t $(TARGET_SELCC)

$(LOGDIR)/%_selcc.log: $(BINDIR)/%_selcc.bin
	@echo Simulating $<
	@cd ..;\
	$(SIM) -i examples/$< -o examples/$@ -t $(TARGET_SELCC) -s

# Rules for MovCC target
$(ASMDIR)/%_movcc.s: %.ll
	@echo Creating $@...
	@$(LLC) $(LLC_MOVCC_FLAGS) -o $@ $<

$(BINDIR)/%_movcc.bin: $(ASMDIR)/%_movcc.s
	@echo Assembling $<
	@cd ..;\
	$(ASM) -i examples/$< -o examples/$@ -t $(TARGET_MOVCC)

$(LOGDIR)/%_movcc.log: $(BINDIR)/%_movcc.bin
	@echo Simulating $<
	@cd ..;\
	$(SIM) -i examples/$< -o examples/$@ -t $(TARGET_MOVCC) -s

# Rules for PredBlockICC target
$(ASMDIR)/%_predblockicc.s: %.ll
	@echo Creating $@...
	@$(LLC) $(LLC_PREDBLOCKICC_FLAGS) -o $@ $<

$(BINDIR)/%_predblockicc.bin: $(ASMDIR)/%_predblockicc.s
	@echo Assembling $<
	@cd ..;\
	$(ASM) -i examples/$< -o examples/$@ -t $(TARGET_PREDBLOCKICC)

$(LOGDIR)/%_predblockicc.log: $(BINDIR)/%_predblockicc.bin
	@echo Simulating $<
	@cd ..;\
	$(SIM) -i examples/$< -o examples/$@ -t $(TARGET_PREDBLOCKICC) -s

# Rules for HWLoop target
$(ASMDIR)/%_hwloop.s: %.ll
	@echo Creating $@...
	@$(LLC) $(LLC_HWLOOP_FLAGS) -o $@ $<

$(BINDIR)/%_hwloop.bin: $(ASMDIR)/%_hwloop.s
	@echo Assembling $<
	@cd ..;\
	$(ASM) -i examples/$< -o examples/$@ -t $(TARGET_HWLOOP)

$(LOGDIR)/%_hwloop.log: $(BINDIR)/%_hwloop.bin
	@echo Simulating $<
	@cd ..;\
	$(SIM) -i examples/$< -o examples/$@ -t $(TARGET_HWLOOP) -s


.PHONY: clean
clean:
	@echo Cleaning all assembler files...
	@rm -f $(V8_SFILES) $(SELCC_SFILES) $(MOVCC_SFILES) $(PREDBLOCKICC_SFILES) $(HWLOOP_SFILES)
	@echo Cleaning all binary files...
	@rm -f $(V8_BINFILES) $(SELCC_BINFILES) $(MOVCC_BINFILES) $(PREDBLOCKICC_BINFILES) $(HWLOOP_BINFILES)
	@echo Cleaning all log files...
	@rm -f $(V8_LOGFILES) $(SELCC_LOGFILES) $(MOVCC_LOGFILES) $(PREDBLOCKICC_LOGFILES) $(HWLOOP_LOGFILES)
	@echo Done!


