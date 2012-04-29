CDIR=cfiles
LLDIR=llvmfiles
ASMDIR=asmfiles
INCDIR=include
BINDIR=binfiles
LOGDIR=logfiles

SRCFILES=bs.c bs_sp.c median.c median_torben.c median_sp.c bgcd.c \
bgcd_sp.c dijkstra.c dijkstra_wcet.c dijkstra_sp.c interpolate.c \
interpolate_sp.c swdivision.c swdivision_shift.c swdivision_sp.c \
cover.c bubble_sort.c fdct.c shellsort.c shellsort_sp.c matrixsum.c \
threshold.c threshold_sp.c
#SRCFILES=bs.c bs_sp.c median.c median_torben.c median_sp.c bgcd.c \
bgcd_sp.c dijkstra.c dijkstra_wcet.c dijkstra_sp.c interpolate.c \
interpolate_sp.c swdivision.c swdivision_shift.c swdivision_sp.c
#SRCFILES=swdivision_sp.c
CFILES=$(addprefix $(CDIR)/, $(SRCFILES)) 
LLFILES=$(SRCFILES:.c=.ll)
SFILES=$(addprefix $(ASMDIR)/, $(SRCFILES:.c=.s))
BINFILES=$(addprefix $(BINDIR)/, $(SRCFILES:.c=.bin))
LOGFILES=$(addprefix $(LOGDIR)/, $(SRCFILES:.c=.log))

LLVMDIR=$(HOME)/llvm/llvm-play/Release/bin
LLVMC=$(LLVMDIR)/llvmc
LLC=$(LLVMDIR)/llc

#TARGET=v8
#TARGET=v8-blockicc-movcc
#TARGET=v8-blockpreg-selcc
TARGET=v8-blockicc-selcc
#FEATURES=-mattr=-singleloop,-movcc
FEATURES=

LLVMC_FLAGS=-S -emit-llvm -O3 -I$(INCDIR)
LLC_FLAGS=-march=cbg -mcpu=$(TARGET) $(FEATURES) -filetype=asm

ASM=./assembler
SIM=./simulator


all: $(LOGFILES)

fresh: clean all

.PRECIOUS: $(SFILES) $(BINFILES)

%.ll: $(CDIR)/%.c
	@echo Creating $@...
	@$(LLVMC) $(LLVMC_FLAGS) -o $@ $<

$(ASMDIR)/%.s: %.ll
	@echo Creating $@...
	@$(LLC) $(LLC_FLAGS) -o $@ $<

$(BINDIR)/%.bin: $(ASMDIR)/%.s
	@echo Assembling $<
	@cd ..;\
	$(ASM) -i benchmarks/$< -o benchmarks/$@ -t $(TARGET)

$(LOGDIR)/%.log: $(BINDIR)/%.bin
	@echo Simulating $<
	@cd ..;\
	$(SIM) -i benchmarks/$< -o benchmarks/$@ -t $(TARGET) -s

.PHONY: clean
clean:
	@echo Cleaning all assembler files...
	@rm -f $(SFILES)
	@echo Cleaning all binary files...
	@rm -f $(BINFILES)
	@echo Cleaning all log files...
	@rm -f $(LOGFILES)
	@echo Done!

	

