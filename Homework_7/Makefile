##################################
# Makefile
# # Beckham Carver
# # COSC 3750, Spring 2024
# # Homework 7
# # 4/11/2023
# #
# # Makefile for HW7
# ##################################


CC=gcc
CFLAGS=-ggdb -Wall

# reference to clean
.PHONY: clean

wytalkC: wytalkD wytalkC.c socketfun.c
	${CC} ${CFLAGS} wytalkC.c socketfun.c -o wytalkC

wytalkD: wytalkD.c socketfun.c
	${CC} ${CFLAGS} wytalkD.c socketfun.c -o wytalkD

# remove
clean:
	-rm -f wycat wytalkC wytalkD
