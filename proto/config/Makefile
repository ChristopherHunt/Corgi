CC = mpiicc
LD = mpiicc
HOST_FILE = hosts.txt
EXEC = test

build: test.c
	@echo "Cleaning out directory"
	make clean
	@echo
	@echo "Compiling test MPI program"
	$(CC) test.c -o $(EXEC)
	@echo
	@echo "Running test MPI program on 2 nodes"
	make run

# Run a test to ensure that the nodes are up.
run:
	mpirun -n 4 -ppn 1 -iface eth0 -f $(HOST_FILE) ./$(EXEC)

.PHONY: clean

clean:
	rm -rf $(EXEC) *.o
