# SPARC V8 Instruction Set Extension Simulator
#
# File: Makefile
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
 
include Makefile.mk

SRC=./src
INCLUDE=./include
DEPPATH=./deps
OBJDIR=./obj
LDIR=./shared
YYDIR=./yy

ASMOBJS=$(ASMCFILES:.c=.o) 
ASMOBJFILES=$(addprefix $(OBJDIR)/, $(ASMOBJS))
ASMDEPS=$(addprefix $(DEPPATH)/, $(ASMCFILES:.c=.d))

SIMOBJS=$(SIMCFILES:.c=.o) 
SIMOBJFILES=$(addprefix $(OBJDIR)/, $(SIMOBJS))
SIMDEPS=$(addprefix $(DEPPATH)/, $(SIMCFILES:.c=.d))

YYOBJS=$(YYCFILES:.c=.o)
YYOBJFILES=$(addprefix $(OBJDIR)/, $(YYOBJS))

ASM=assembler
SIM=simulator

vpath %.l $(YYDIR)
vpath %.y $(YYDIR)
vpath %.c $(SRC)
vpath %.tab.c $(YYDIR)
vpath %.flex.c $(YYDIR)
vpath %.h $(INCLUDE)
vpath %.o $(OBJDIR)
vpath %.so $(LDIR)

CC=gcc
CFLAGS=-Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-function -Wno-implicit-function-declaration -std=c99
IFLAGS=-I$(INCLUDE)
ASMLFLAGS=-ly -ll -ldl
SIMLFLAGS=-ldl
#DBG=-ggdb -DSIM_DBG
DBG=

all: $(ASM) $(SIM) $(SHOBJS)
	@echo Checking for shared libraries...
	@cd $(LDIR); make all
	
include $(ASMDEPS)
include $(SIMDEPS)


$(ASM): $(ASMOBJS) $(YYOBJS)
	@echo Linking all object files for assembler...
	@$(CC) -o $(ASM) $(ASMOBJFILES) $(YYOBJFILES) $(ASMLFLAGS)
	@echo Done!

$(ASMOBJS): $(addprefix $(YYDIR)/, $(YACCFILE))

$(SIM): $(SIMOBJS) 
	@echo Linking all object files for simulator...
	@$(CC) -o $(SIM) $(SIMOBJFILES) $(SIMLFLAGS)
	@echo Done!

$(YYINCLUDEFILE): $(addprefix $(YYDIR)/, $(YACCCFILE))

%.tab.c: %.y
	@echo Creating yacc C output files...
	@yacc -d -b $(YYPREFIX) $< 
	@mv $(notdir $@) $@
	@mv $(YYINCLUDEFILE) $(INCLUDE)/$(YYINCLUDEFILE) 

%.tab.o: %.tab.c
	@echo Compiling $<...
	@$(CC) $(CFLAGS) $(DBG) $(IFLAGS) -o $(OBJDIR)/$@ -c $< 

%.flex.c: %.l
	@echo Creating flex C output files...
	@flex -o $@ $< 

%.flex.o: %.flex.c
	@echo Compiling $<...
	@$(CC) $(CFLAGS) $(DBG) $(IFLAGS) -o $(OBJDIR)/$@ -c $< 

$(DEPPATH)/%.d: %.c
	@echo Creating dependency file $@...
	@$(CC) -MM -MG $< -MT "$@ $(addprefix $(OBJDIR)/, $(@:.d=.o))" > $@

%.o: %.c $(DEPPATH)/%.d
	@echo Compiling $<...
	@$(CC) $(CFLAGS) $(DBG) $(IFLAGS) -o $(OBJDIR)/$@ -c $< 

distclean: clean
	@cd $(LDIR); make clean
	@echo Removing temporary object and generated header files.
	@rm -f $(ASMOBJFILES) 
	@rm -f $(SIMOBJFILES) 
	@rm -f $(addprefix $(OBJDIR)/, $(YYOBJS))
	@rm -f $(addprefix $(YYDIR)/, $(YYCFILES))
	@rm -f $(INCLUDE)/$(YYINCLUDEFILE)

.PHONY: depclean
depclean:
	@rm -f $(ASMDEPS)
	@rm -f $(SIMDEPS)

clean:
	@echo Removing $(ASM).
	@rm -f $(ASM)
	@echo Removing $(SIM).
	@rm -f $(SIM)
