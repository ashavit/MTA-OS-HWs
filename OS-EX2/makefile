CC = gcc
STD = -std=c99
CFLAGS = -c -Wall

OS:=$(shell uname)
ifneq (,$(findstring CYGWIN_NT,$(OS)))
  EXT=.exe
else
  EXT=
endif

PROG_NAME=ex2_q1
PROG=$(PROG_NAME)$(EXT)

All: $(PROG) test

$(PROG): $(PROG_NAME).o PolygonInterface.o Workers.o
	$(CC) $(STD) $(PROG_NAME).o PolygonInterface.o Workers.o -o $(PROG)

$(PROG_NAME).o: $(PROG_NAME).c PolygonInterface.h
	$(CC) $(STD) $(CFLAGS) $(PROG_NAME).c

PolygonInterface.o: PolygonInterface.c PolygonInterface.h Workers.h Commons.h
	$(CC) $(STD) $(CFLAGS) PolygonInterface.c

Workers.o: Workers.c Workers.h Commons.h
	$(CC) $(STD) $(CFLAGS) Workers.c

test: $(PROG)
	./$(PROG) < in.txt > out.log 2> err.log

clean:
	rm -vrdf *.o $(PROG) *.log
