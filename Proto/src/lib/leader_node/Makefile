CC = mpic++
LD = mpic++
UTILS = ../utils

EXECS = leader_node_main

build: leader_node.cpp leader_node_main.cpp $(UTILS)/network_header.cpp
	$(CC) leader_node.cpp leader_node_main.cpp $(UTILS)/network_header.cpp -o leader_node_main

.PHONY: clean

clean:
	rm -f *.o $(EXECS)
