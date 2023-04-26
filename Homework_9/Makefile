##################################
# Makefile
# # Beckham Carver
# # COSC 3750, Spring 2024
# # Homework 8
# # 4/18/2023
# #
# # Makefile for HW8
# ##################################


CC=gcc
CFLAGS=-ggdb -Wall

# reference to clean
.PHONY: clean

wyshell: wyshell.c wyscanner.c
	${CC} ${CFLAGS} wyshell.c wyscanner.c -o wyshell

# remove
clean:
	-rm -f wyshell
