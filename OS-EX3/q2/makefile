CC = gcc
STD = -std=c99
CFLAGS = -c -Wall

OS:=$(shell uname)
ifneq (,$(findstring CYGWIN_NT,$(OS)))
  EXT=.exe
else
  EXT=
endif

PROG_NAME=ex3_q2
PROG=$(PROG_NAME)$(EXT)

All: $(PROG) test

$(PROG): $(PROG_NAME).o
	$(CC) $(STD) $(PROG_NAME).o -o $(PROG) -l pthread

$(PROG_NAME).o: $(PROG_NAME).c
	$(CC) $(STD) $(CFLAGS) $(PROG_NAME).c

test1: $(PROG) $(WRITER_PROG)
	./$(PROG) shopper.txt1

test1_color: $(PROG) $(WRITER_PROG)
	./$(PROG) shopper.txt1 | grep --color fitting

test2: $(PROG) $(WRITER_PROG)
	./$(PROG) shopper.txt2

test2_color: $(PROG) $(WRITER_PROG)
	./$(PROG) shopper.txt2 | grep --color fitting

test3: $(PROG) $(WRITER_PROG)
	./$(PROG) shopper.txt3

test3_color: $(PROG) $(WRITER_PROG)
	./$(PROG) shopper.txt3 | grep --color fitting

test: $(PROG) $(WRITER_PROG)
	./$(PROG) shopper.txt1 > shopper.log1 2> shopper.err1
	./$(PROG) shopper.txt2 > shopper.log2 2> shopper.err2
	./$(PROG) shopper.txt3 > shopper.log3 2> shopper.err3

clean:
	rm -vrdf *.o $(PROG) *.log* *.err*
