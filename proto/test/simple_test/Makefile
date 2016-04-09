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
SRC := simple_test.cpp

.PHONY: build build-lib build-src run clean fullclean

build:
	$(MAKE) -s -C $(GTEST_DIR) build
	$(CXX) -isystem $(GTEST_DIR)/include -I$(GTEST_DIR) -pthread \
		$(SRC) $(GTEST_DIR)/libgtest.a -o $(TEST)
	
run: clean build
	./$(TEST)

clean:
	$(RM) -rf *.o $(TEST)

fullclean: clean
	$(MAKE) -s -C $(GTEST_DIR) clean
