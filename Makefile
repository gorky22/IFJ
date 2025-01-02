# IFJ Compiler project - Makefile
# Team Leader:
# Team Members:

# == Variables ================================================================

CC=gcc
CFLAGS=-std=c11 -pedantic -Werror -Wextra -Wall -O2
LFLAGS=-lm #if we would need to use math library

ZIPNAME=ifjteam
PROG=compiler

SRC=./src

SOURCES=$(wildcard $(SRC)/**/*.c $(SRC)/*.c)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))
# linking objects like grammar.o, parser.o etc.
HEADERS=$(wildcard $(SRC)/**/*.h $(SRC)/*.h)
# for each object use specific name of header and not the HEADERS variable

# == Debugging ================================================================

DEBUG=0

ifeq ($(DEBUG), 1)
CFLAGS += -g -DDEBUG
endif

# == Executables ==============================================================

all: $(PROG) clear

# == Objects ==================================================================
# $@ == main or anything before :
$(PROG): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

./src/parser.o: ./src/parser.c
	$(CC) $(CFLAGS) -Wno-implicit-fallthrough -Wno-switch $^ -c -o $@

%.o: %.c #$(patsubst %.c, %.h, $<)
	$(CC) $(CFLAGS) $^ -c -o $@

.PHONY: all clean

# == Utilities ================================================================

cleanup:
	rm $(PROG)
	rm output.txt
	rm *.zip

clear:
	rm src/*.o

pack:
	zip $(ZIPNAME).zip src/*.c src/*.h Makefile
