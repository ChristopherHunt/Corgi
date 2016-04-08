SHELL := /bin/bash

MPICH = $(shell command -v mpic++)
MPIICC = $(shell command -v mpiicc)

ifdef MPICH
CXX := mpic++
else
ifdef MPIICC
CXX := mpiicc
else
$(error Neither Intel MPI nor MPICH installed, please install either)
endif
endif

CXXFLAGS := -O3 -std=c++0x

INCLUDES := -I../../lib/

OUTFILE := $(BIN) $(LIB)

.PHONY: default clean

default: $(OUTFILE)

clean:
	$(RM) *.o
	$(RM) $(OUTFILE)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CXX_DEBUG_FLAGS) -o $@ -c $< $(INCLUDES)
