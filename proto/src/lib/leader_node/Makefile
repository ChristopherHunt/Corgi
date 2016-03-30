CC = mpiicc
CFLAGS = -std=c++0x
LD = mpiicc
UTILS = ../utils

EXECS = leader_node_main

build: leader_node.cpp leader_node_main.cpp $(UTILS)/network_header.cpp
	$(CC) $(CFLAGS) leader_node.cpp leader_node_main.cpp $(UTILS)/network_header.cpp -o leader_node_main

debug: leader_node.cpp leader_node_main.cpp $(UTILS)/network_header.cpp
	$(CC) $(CFLAGS) -D DEBUG leader_node.cpp leader_node_main.cpp $(UTILS)/network_header.cpp -o leader_node_main

.PHONY: clean

clean:
	rm -f *.o $(EXECS)
