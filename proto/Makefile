SHELL := /bin/bash
host_file := config/hosts.txt

lib_cache_api := src/lib/cache_api
lib_leader_node := src/lib/leader_node
lib_swing_node := src/lib/swing_node
lib_cache_node := src/lib/cache_node
lib_policy := src/lib/policy
lib_utils := src/lib/utils

libraries := $(lib_cache_api) $(lib_leader_node) $(lib_swing_node)\
			 $(lib_cache_node) $(lib_policy) $(lib_utils) 

leader_layer := src/app/leader_layer
swing_layer := src/app/swing_layer
cache_layer := src/app/cache_layer
job_layer := src/app/job_layer
 
apps := $(leader_layer) $(swing_layer) $(cache_layer) $(job_layer)

dirs := $(libraries) $(apps)

.PHONY: build cp_bins run $(apps) $(libraries)

build: $(apps) cp_bins

$(apps): $(libraries)
	$(MAKE) -s -C $@

$(libraries):
	$(MAKE) -s -C $@
	mkdir -p lib
	cp $@/*.a lib

cp_bins:
	mkdir -p bin
	cp src/app/leader_layer/leader_layer bin
	cp src/app/swing_layer/swing_layer bin
	cp src/app/cache_layer/cache_layer bin
	cp src/app/job_layer/job_layer bin

run: build
	#cd bin && mpirun -n 4 -ppn 1 -iface eth0 -f $(host_file) ./leader_layer
	cd bin && mpirun -n 1 ./leader_layer

clean:
	$(RM) -rf bin
	$(RM) -rf lib
	for DIR in ${dirs}; do $(MAKE) -s -C $${DIR} clean; done
