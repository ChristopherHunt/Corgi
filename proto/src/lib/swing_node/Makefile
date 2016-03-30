CC = mpiicc
CFLAGS = -std=c++0x
LD = mpiicc
UTILS = ../utils
POLICY = ../policy

EXECS = swing_node_main

build: swing_node.cpp swing_node_main.cpp $(UTILS)/network_header.cpp $(POLICY)/quorum.cpp
	$(CC) $(CFLAGS) swing_node.cpp swing_node_main.cpp $(UTILS)/network_header.cpp $(POLICY)/quorum.cpp -o swing_node_main

debug: swing_node.cpp swing_node_main.cpp $(UTILS)/network_header.cpp $(POLICY)/quorum.cpp
	$(CC) $(CFLAGS) -D DEBUG swing_node.cpp swing_node_main.cpp $(UTILS)/network_header.cpp $(POLICY)/quorum.cpp -o swing_node_main

.PHONY: clean

clean:
	rm -f *.o $(EXECS)
