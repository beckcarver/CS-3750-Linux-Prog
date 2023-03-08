##################################
# Makefile
# Beckham Carver
# COSC 3750, Spring 2024
# Homework 5
# 03/07/2023
#
# Makefile for HW5
##################################


CC=gcc
CFLAGS=-g -Wall -I.

# reference to clean and tidy recipes
.PHONY: clean tidy

# create approxe using approxe.c, compile with CC/CFLAGS + '-I'
wycat: wyls.c
	${CC} ${CFLAGS} wyls.c -o wyls

# remove OBJS, and approxe
tidy:
	-rm -f wyls core
clean: tidy
