# Author: Aidan Bush
# Assign: Assign 1
# Course: CMPT 361
# Date: Sept. 19, 17
# File: Makefile
# Description: it's a Makefile

CC=gcc
CFLAGS= -Wall -std=c99 -D_POSIX_C_SOURCE=201112L -g

.PHONEY: all clean

all: qotdd

qotdd: qotdd.o child_proc.o host_info.o

qotdd.o: qotdd.c child_proc.h host_info.h

child_proc.o: child_proc.c

host_info.o: host_info.c host_info.h

# add clean for jsmn
clean:
	$(RM) qotdd *.o
