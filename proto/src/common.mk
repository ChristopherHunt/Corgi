SHELL := /bin/bash

mpich = $(shell command -v mpic++)
mpiicc = $(shell command -v mpiicc)

ifdef mpich
CXX := mpic++
else
ifdef mpiicc
CXX := mpiicc
else
$(error Neither Intel MPI nor MPICH installed, please install either.)
endif
endif

CXXFLAGS += -O3 -std=c++0x -Wall

includes += -I$(base_dir)/src/lib/

to_build := $(app) $(lib) $(test)

.PHONY: default clean

default: $(to_build)

clean:
	$(RM) *.o *.log
	$(RM) $(to_build)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(cxx_debug_flags) -o $@ -c $< $(includes)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(cxx_debug_flags) -o $@ -c $< $(includes)
