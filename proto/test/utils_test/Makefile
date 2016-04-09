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

# Modify these variables to setup a test
TEST := utils_test# Name of the test executable to be created
SRC := utils_test.cpp # Name of the src executable to be compiled 
APP_LIBS := lib_utils.a	# List of all the archives this test needs to reference
# Leave the rest alone

CXXFLAGS := -O3 -std=c++0x -pthread

GTEST_DIR := ../googletest
USER_INCLUDE := ../../src/lib/
LDLIBS := $(addprefix ../../lib/, $(APP_LIBS))

.PHONY: all build build-lib build-src run clean fullclean

all: clean build run

build:
	$(MAKE) -s -C $(GTEST_DIR) build
	$(CXX) -I$(USER_INCLUDE) -isystem $(GTEST_DIR)/include -I$(GTEST_DIR) \
		$(LDLIBS) $(SRC) $(GTEST_DIR)/libgtest.a -o $(TEST)
	
run:
	./$(TEST)

clean:
	$(RM) -rf *.o $(TEST)

fullclean: clean
	$(MAKE) -s -C $(GTEST_DIR) clean
