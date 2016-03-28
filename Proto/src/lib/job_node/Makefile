CC = mpic++
LD = mpic++
UTILS = ../utils
API = ../cache_api

EXECS = job_node_main 

build: job_node_main.cpp $(UTILS)/network_header.cpp $(API)/cache.cpp
	$(CC) job_node_main.cpp $(UTILS)/network_header.cpp $(API)/cache.cpp -o job_node_main

.PHONY: clean

clean:
	rm -f *.o $(EXECS)
