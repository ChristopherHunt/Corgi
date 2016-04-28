SHELL := /bin/bash

CXXFLAGS += -O3 -std=c++0x -Wall

includes += -I$(base_dir)/src/lib/

to_build := $(app) $(lib) $(test)

.PHONY: default clean

default: $(to_build)

clean:
	$(RM) *.o *.log
	$(RM) $(to_build)

%.o: %.cpp
	$(MPICXX) $(CXXFLAGS) $(cxx_debug_flags) -o $@ -c $< $(includes)

%.o: %.cc
	$(MPICXX) $(CXXFLAGS) $(cxx_debug_flags) -o $@ -c $< $(includes)
