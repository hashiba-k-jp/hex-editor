# Makefile
CC = clang
OUTNAME = hex

editor: hex.c config.h terminalMode.h keys.h
	# https://qiita.com/kaityo256/items/80863d466e0d69808eb9
	$(CC) -g -O0 hex.c -o $(OUTNAME)


test:editor hex
	./hex

clean:
	rm -f $(OUTNAME)
