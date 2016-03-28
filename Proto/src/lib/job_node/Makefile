CC = mpic++
LD = mpic++
UTILS = ../utils

EXECS = job_node_main 

build: job_node_main.cpp $(UTILS)/network_header.cpp
	$(CC) job_node_main.cpp $(UTILS)/network_header.cpp -o job_node_main

.PHONY: clean

clean:
	rm -f *.o $(EXECS)
