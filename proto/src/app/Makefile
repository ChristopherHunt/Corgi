CC = mpiicc
LD = mpiicc

EXECS = leader_node_main swing_node_main cache_node_main job_node_main

run:
	mpirun -n 1 ./leader_node_main -iface eth0

.PHONY: clean

clean:
	rm -f $(EXECS)
