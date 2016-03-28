CC = mpic++
LD = mpic++

APP_DIR = ../app

build:
	make clean
	make leader_node_main
	make swing_node_main
	make cache_node_main
	make job_node_main

leader_node_main:
	make -C leader_node build
	mv leader_node/leader_node_main ../app

swing_node_main:
	make -C swing_node build
	mv swing_node/swing_node_main ../app

cache_node_main:
	make -C cache_node build
	mv cache_node/cache_node_main ../app

job_node_main:
	make -C job_node build
	mv job_node/job_node_main ../app

.PHONY: clean

clean:
	make -C leader_node clean
	make -C swing_node clean
	make -C cache_node clean
	make -C job_node clean
