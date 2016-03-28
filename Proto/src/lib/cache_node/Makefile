CC = mpic++
LD = mpic++
UTILS = ../utils
POLICY = ../policy

EXECS = cache_node_main

build: cache_node.cpp cache_node_main.cpp $(UTILS)/network_header.cpp $(POLICY)/quorum.cpp
	$(CC) cache_node.cpp cache_node_main.cpp $(UTILS)/network_header.cpp $(POLICY)/quorum.cpp -o cache_node_main

.PHONY: clean

clean:
	rm -f *.o $(EXECS)
