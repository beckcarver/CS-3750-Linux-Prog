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

CC = gcc
CFLAGS = -ggdb -Wall
OBJS = prompt.o compute.o display.o

# reference to clean and tidy recipes
.PHONY: clean tidy

# create compute.o using compute files, compile with CC/FLAGS + '-c'
compute.o: compute.c compute.h
	${CC} ${CFLAGS} -c compute.c compute.h

# create display.o using display files, compile with CC/CFLAGS + '-c'
display.o: display.c display.h
	${CC} ${CFLAGS} -c display.c display.h

# create prompt.o using prompt files, compile with CC/CFLAGS + '-c'
prompt.o: prompt.c prompt.h
	${CC} ${CFLAGS} -c prompt.c prompt.h

# create approxe using approxe.c, compile with CC/CFLAGS + '-I'
approxe: ${OBJS} approxe.c
	${CC} ${CFLAGS} -Iinclude ${OBJS} approxe.c -o approxe


# remove OBJS, and approxe
tidy:
	/bin/rm -f ${OBJS}
clean: tidy
	/bin/rm -f ${OBJS} approxe
