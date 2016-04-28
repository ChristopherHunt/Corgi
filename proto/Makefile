SHELL := /bin/bash
host_file := config/hosts.txt
base_dir := $(CURDIR)

lib_dir := $(base_dir)/lib
log_dir := $(base_dir)/log
bin_dir := $(base_dir)/bin
bin_test_dir := $(bin_dir)/test

export base_dir
export lib_dir
export log_dir
export bin_dir
export bin_test_dir
export cxx_debug_flags

# Enumeration of all of the libraries to compile for the project
lib_cache_api := src/lib/cache_api
lib_leader_node := src/lib/leader_node
lib_swing_node := src/lib/swing_node
lib_cache_node := src/lib/cache_node
lib_cache_quorum := src/lib/policy/quorum/cache
lib_swing_quorum := src/lib/policy/quorum/swing
lib_network := src/lib/network
lib_logger := src/lib/utility/logger
lib_utils := src/lib/utility/utils
lib_gtest := src/test/googletest

# Enumeration of all of the tests to run for the project
test_utils := src/test/utils_test
test_logger := src/test/logger_test
test_put_get_local := src/test/put_get_local_test
test_push_pull_local := src/test/push_pull_local_test

# Enumeration of all of the executables for the project
leader_layer := src/app/leader_layer
swing_layer := src/app/swing_layer
cache_layer := src/app/cache_layer
job_layer := src/app/job_layer

# List containing all of the user libraries for the project
libraries := $(lib_cache_api) $(lib_leader_node) $(lib_swing_node)\
			 $(lib_cache_node) $(lib_cache_quorum) $(lib_swing_quorum)\
			 $(lib_network) $(lib_logger) $(lib_utils) $(lib_gtest)

# List containing all of the user tests for the project
tests := $(test_utils) $(test_logger) $(test_put_get_local)\
		   $(test_push_pull_local)

# List containing all of the test applications used for testing this project
test_apps := $(put_local_app)

# List of all of the executables for the project
apps := $(leader_layer) $(swing_layer) $(cache_layer) $(job_layer)

# List of all directories containing Makefiles to build
dirs := $(libraries) $(apps) $(tests)

.PHONY: all build dirs run $(dirs) $(apps) $(libraries) $(tests)

all: build run

dirs:
	mkdir -p $(lib_dir)
	mkdir -p $(bin_dir)
	mkdir -p $(bin_test_dir)
	mkdir -p $(log_dir)

build: clean dirs $(apps) $(tests)

$(apps): $(libraries)
	$(MAKE) -s -C $@

$(libraries):
	$(MAKE) -s -C $@

$(tests): $(test_apps)
	#$(MAKE) -s -C $@

debug:
	$(eval cxx_debug_flags += -D DEBUG)
	$(MAKE) build

run:
	#cd bin && mpirun -n 1 -ppn 1 -iface eth0 -f ../$(host_file) ./leader_layer ./job_layer
	cd bin && mpirun -n 1 ./leader_layer ./job_layer 2>&1 | tee $(log_dir)/run.log

clean:
	$(RM) -rf $(lib_dir)
	$(RM) -rf $(bin_test_dir)
	$(RM) -rf $(bin_dir)
	$(RM) -rf $(log_dir)
	for DIR in ${dirs}; do $(MAKE) -s -C $${DIR} clean; done
