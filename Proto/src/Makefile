CC = mpic++
LD = mpic++

APP_DIR = app

build:
	make clean
	make -C lib build
	make -C app run

.PHONY: clean

clean:
	make -C lib clean
	make -C app clean
