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
GTEST_DIR := .

.PHONY: build clean

build:
	$(CXX) -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c \
		${GTEST_DIR}/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o

clean:
	$(RM) -rf *.a *.o
