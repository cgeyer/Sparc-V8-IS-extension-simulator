# SPARC V8 Instruction Set Extension Simulator
#
# File: shared/Makefile
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

include ../Makefile.mk

INCLUDE=../include

OBJS=$(SHCFILES:.c=.o)
DEPS=$(SHCFILES:.c=.d)

vpath %.h $(INCLUDE)

CC=gcc
CFLAGS=-Wall -Wextra -Werror -Wno-unused-parameter -fPIC -std=c99
LFLAGS=-shared 
IFLAGS=-I$(INCLUDE)

all: $(SHARED_OBJS)

include $(DEPS)

%.d: %.c
	@echo Creating dependency file $@...
	@$(CC) -MM -MG $< -MT "$@ $(<:.c=.o)" > $@

%.so: %.o
	@echo Linking $<...
	@$(CC) $(LFLAGS) -o $@ $<
	@echo Done!
		
%.o: %.c
	@echo Compiling $<...
	@$(CC) $(CFLAGS) $(IFLAGS) -c $< 

clean:
	@echo Removing $(SHARED_OBJS).
	@rm -f $(SHARED_OBJS)
	@echo Removing $(OBJS).
	@rm -f $(OBJS)
