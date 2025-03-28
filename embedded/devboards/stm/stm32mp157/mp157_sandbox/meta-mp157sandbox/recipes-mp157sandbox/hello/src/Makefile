# setting CC breaks yocto
# ifdef CROSS_COMPILE
# CC := ${CROSS_COMPILE}gcc
# else
# CC := gcc
# endif

.DEFAULT_GOAL := hello

hello.o:
	${CC} -c hello.c

.PHONY: all clean

all: hello

clean:
	rm -f hello.o hello
