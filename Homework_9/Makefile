##################################
# Makefile
# # Beckham Carver
# # COSC 3750, Spring 2024
# # Homework 8
# # 4/25/2023
# #
# # Makefile for HW9
# ##################################


CC=gcc
CFLAGS=-ggdb -Wall -Wno-unused-function

# reference to clean
.PHONY: clean

wyshell: wyshell.c wyscanner.c wyargs.c
	${CC} ${CFLAGS} wyshell.c wyscanner.c wyargs.c -o wyshell

# remove
clean:
	-rm -f wyshell
