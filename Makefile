# Author: Aidan Bush
# Assign: Assign 1
# Course: CMPT 361
# Date: Sept. 19, 17
# File: Makefile
# Description: it's a Makefile

CC=gcc
CFLAGS= -Wall -std=c99 -D_POSIX_C_SOURCE=201112L -pedantic

.PHONEY: all clean

all: qotdd

qotdd: qotdd.o child_proc.o host_info.o jsmn/jsmn.o

qotdd.o: qotdd.c child_proc.h host_info.h

child_proc.o: child_proc.c host_info.h jsmn/jsmn.h

host_info.o: host_info.c host_info.h

jsmn/jsmn.o:
	make -C jsmn

clean:
	$(RM) qotdd *.o
	make -C jsmn clean
