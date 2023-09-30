# Makefile
CC = gcc

#include "terminalmode.h"  // require no other files.
#include "editorconfig.h"  // require no other files.
#include "terminalprint.h" // require editorconfig.h
#include "cursor.h"        // require editorconfig.h
#include "editdata.h"      // require editorconfig.h and cursor.h

editor:hex.c terminalmode.h editorconfig.h terminalprint.h cursor.h editdata.h
	$(CC) -Wall -save-temps -o hex hex.c

test:editor hex
	./hex test.txt -c

clean:
	rm -f hex
