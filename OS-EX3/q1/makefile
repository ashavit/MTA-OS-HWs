CC = gcc
STD = -std=c99
CFLAGS = -c -Wall

OS:=$(shell uname)
ifneq (,$(findstring CYGWIN_NT,$(OS)))
  EXT=.exe
else
  EXT=
endif

PROG_NAME=ex3_q1
PROG=$(PROG_NAME)$(EXT)
WRITER_NAME=writer
WRITER_PROG=$(WRITER_NAME)$(EXT)

All: $(PROG) $(WRITER_PROG) test

$(PROG): $(PROG_NAME).o
	$(CC) $(STD) $(PROG_NAME).o -o $(PROG)

$(WRITER_PROG): $(WRITER_NAME).o
	$(CC) $(STD) $(WRITER_NAME).o -o $(WRITER_PROG)

$(PROG_NAME).o: $(PROG_NAME).c Commons.h
	$(CC) $(STD) $(CFLAGS) $(PROG_NAME).c

$(WRITER_NAME).o: $(WRITER_NAME).c Commons.h
	$(CC) $(STD) $(CFLAGS) $(WRITER_NAME).c

writer_test: $(WRITER_PROG)
		./$(WRITER_PROG) hello < in.txt > writer_test.log

test: $(PROG) $(WRITER_PROG)
	./$(PROG) 10 out.log < in.txt 2> err.log

clean:
	rm -vrdf *.o $(PROG) $(WRITER_PROG) *.log
