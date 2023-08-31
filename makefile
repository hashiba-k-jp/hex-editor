# Makefile
CC = gcc

editor: hex.c
	$(CC) hex.c -o hex

test:
	make
	./hex test.txt

clean:
	rm -f hex
