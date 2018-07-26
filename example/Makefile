
all: lib example

include ../Makefile.inc

DEPS = ../src/libflexop.a

example.o: example.c $(DEPS)

lib:
	@(cd ../src; make)

clean:
	rm -fv *.o core.* example

