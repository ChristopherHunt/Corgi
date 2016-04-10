SHELL := /bin/bash
host_file := config/hosts.txt

export CXX_DEBUG_FLAGS

# Enumeration of all of the libraries to compile for the project
lib_cache_api := src/lib/cache_api
lib_leader_node := src/lib/leader_node
lib_swing_node := src/lib/swing_node
lib_cache_node := src/lib/cache_node
lib_policy := src/lib/policy
lib_network := src/lib/network
lib_utils := src/lib/utils

# Enumeration of all of the tests to run for the project
test_utils := test/utils_test

# Enumeration of all of the executables for the project
leader_layer := src/app/leader_layer
swing_layer := src/app/swing_layer
cache_layer := src/app/cache_layer
job_layer := src/app/job_layer
 
# List containing all of the user libraries for the project
libraries := $(lib_cache_api) $(lib_leader_node) $(lib_swing_node)\
			 $(lib_cache_node) $(lib_policy) $(lib_network) $(lib_utils)

# List containing all of the user tests for the project
tests := $(test_utils)

# List of all of the executables for the project
apps := $(leader_layer) $(swing_layer) $(cache_layer) $(job_layer)

dirs := $(libraries) $(apps) $(tests)

.PHONY: all build cp_bins dirs run test $(dirs) $(apps) $(libraries) $(tests) 

all: build test run

dirs:
	mkdir -p lib
	mkdir -p bin

test: $(tests)

$(tests): build
	$(MAKE) -s -C $@

build: clean dirs $(apps) cp_bins

debug:
	$(eval CXX_DEBUG_FLAGS += -D DEBUG)
	$(MAKE) build

$(apps): $(libraries)
	$(MAKE) -s -C $@

$(libraries):
	$(MAKE) -s -C $@
	cp $@/*.a lib

cp_bins:
	cp src/app/leader_layer/leader_layer bin
	cp src/app/swing_layer/swing_layer bin
	cp src/app/cache_layer/cache_layer bin
	cp src/app/job_layer/job_layer bin

run:
	#cd bin && mpirun -n 4 -ppn 1 -iface eth0 -f $(host_file) ./leader_layer
	cd bin && mpirun -n 1 ./leader_layer

clean:
	$(RM) -rf bin
	$(RM) -rf lib
	for DIR in ${dirs}; do $(MAKE) -s -C $${DIR} clean; done
