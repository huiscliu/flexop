
.SUFFIXES: .c

.PHONY: clean

default: lib

include ../Makefile.inc
include Makefile.dep

SRC_C   = $(wildcard *.c)
DIR_C   = $(notdir $(SRC_C))
OBJS_C  = $(patsubst %.c,%.o,$(DIR_C))

OBJS   := $(OBJS_C)

lib: libflexop.a
	
libflexop.a: $(OBJS)
	@${AR} -ruv $@ ${OBJS}
	${RANLIB} $@

clean:
	rm -fv libflexop.a *.o 
