##################################
# Makefile
# Beckham Carver
# COSC 3750, Spring 2024
# Homework 6
# 03/28/2023
#
# Makefile for HW6
##################################


CC=gcc
CFLAGS=-ggdb -Wall

# reference to clean and tidy recipes
.PHONY: clean tidy

# create wytar, compile with CC/CFLAGS
wytar: main.o wytar_utils.o
	${CC} ${CFLAGS} -o wytar wytar.c wytar_utils.o -lm

main.o: wytar.c
	${CC} ${CFLAGS} -c wytar_utils.c

wytar_utils.o: wytar_utils.c
	${CC} ${CFLAGS} -c wytar_utils.c

# remove OBJS and wytar
tidy:
	-rm -f wytar core
clean: 
	-rm -f wytar_utils.o main.o
