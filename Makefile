# Author: Aidan Bush
# Assign: Assign 1
# Course: CMPT 361
# Date: Sept. 19, 17
# File: Makefile
# Description: it's a Makefile

CC=gcc
CFLAGS= -Wall -std=c99 -D_POSIX_C_SOURCE=201112L

.PHONEY: all clean

all: qotdd

qotdd: qotdd.o

qotdd.o: qotdd.c

# add clean for jsmn
clean:
	$(RM) qotdd *.o
