# Makefile
CC = clang
OUTNAME = hex

editor: hex.c config.h terminalMode.h keys.h file.h screen.h header.h
	# https://qiita.com/kaityo256/items/80863d466e0d69808eb9
	$(CC) -Ofast hex.c -o $(OUTNAME)


test:editor hex
	./hex

test1:editor hex
	./hex -i 1example.txt

test2:editor hex
	./hex -i 2example.txt

test3:editor hex
	./hex -i 3example.txt

debug:
	$(CC) -g -O0 hex.c -o $(OUTNAME)
	lldb ./hex

clean:
	rm -f $(OUTNAME)
