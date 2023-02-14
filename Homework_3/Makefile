##################################
# Makefile
# Beckham Carver
# COSC 3750, Spring 2024
# Homework 3
# 2/13/2023
#
# Some discussion of what this file is about. Does not have to be
# long winded.
##################################

CC=gcc
CFLAGS=-g -Wall -I.
OBJS=prompt.o compute.o display.o


# reference to clean and tidy recipes
.PHONY: clean tidy

# create approxe using approxe.c, compile with CC/CFLAGS + '-I'
approxe: ${OBJS} approxe.c
	${CC} ${CFLAGS} ${OBJS} approxe.c -o approxe

# create compute.o using compute files, compile with CC/FLAGS + '-c'
compute.o: compute.h compute.c
	${CC} ${CFLAGS} -c compute.c

# create display.o using display files, compile with CC/CFLAGS + '-c'
display.o: display.h display.c
	${CC} ${CFLAGS} -c display.c

# create prompt.o using prompt files, compile with CC/CFLAGS + '-c'
prompt.o: prompt.h prompt.c
	${CC} ${CFLAGS} -c prompt.c


# remove OBJS, and approxe
tidy:
	/bin/rm -f ${OBJS}
clean: tidy
	/bin/rm -f ${OBJS} approxe
