# setting CC breaks yocto
# ifdef CROSS_COMPILE
# CC := ${CROSS_COMPILE}gcc
# else
# CC := gcc
# endif

.DEFAULT_GOAL := bmp180log

bmp180log.o:
	${CC} -c bmp180log.c

.PHONY: all clean

all: bmp180log

clean:
	rm -f bmp180log.o bmp180log
