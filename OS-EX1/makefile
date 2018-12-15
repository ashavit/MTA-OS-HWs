CC = gcc
STD = -std=c99
CFLAGS = -c -Wall

OS:=$(shell uname)
ifneq (,$(findstring CYGWIN_NT,$(OS)))
  EXT=.exe
else
  EXT=
endif

PROG1=ex1_q1$(EXT)

All: $(PROG1) test

$(PROG1): ex1_q1.o PolygonInterface.o
	$(CC) $(STD) ex1_q1.o PolygonInterface.o -o $(PROG1)

ex1_q1.o: ex1_q1.c PolygonInterface.h
	$(CC) $(STD) $(CFLAGS) ex1_q1.c

PolygonInterface.o: PolygonInterface.c PolygonInterface.h Commons.h
	$(CC) $(STD) $(CFLAGS) PolygonInterface.c

test: $(PROG1)
	./$(PROG1) < in.txt > out.log

clean:
	rm -vrdf *.o $(PROG1) *.log
