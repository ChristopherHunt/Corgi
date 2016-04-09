SHELL := /bin/bash

MPICH = $(shell command -v mpic++)
MPIICC= $(shell command -v mpiicc)

ifdef MPICH
CXX := mpic++
else
ifdef MPIICC
CXX := mpiicc
else
$(error Neither Intel MPI nor MPICH installed, please install either.)
endif
endif

CXXFLAGS := -O3 -std=c++0x

GTEST_DIR := ../googletest
TEST := simple_test
SRC := $(TEST).cpp

.PHONY: all build build-lib build-src run clean fullclean

all: clean build run

build: clean
	$(MAKE) -s -C $(GTEST_DIR) build
	$(CXX) -isystem $(GTEST_DIR)/include -I$(GTEST_DIR) -pthread \
		$(SRC) $(GTEST_DIR)/libgtest.a -o $(TEST)
	
run:
	./$(TEST)

clean:
	$(RM) -rf *.o $(TEST)

fullclean: clean
	$(MAKE) -s -C $(GTEST_DIR) clean
