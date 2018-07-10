
all: lib example

include ../Makefile.inc

DEPS = ../src/libflexop.a

SRC   = $(wildcard *.c)
DIR   = $(notdir $(SRC))
OBJS  = $(patsubst %.c,%.o,$(DIR))
BIN   = $(patsubst %.c,%,$(DIR))


example.o: example.c $(DEPS)

lib:
	@(cd ../src; make)

clean:
	rm -fv *.o core.* $(BIN)

