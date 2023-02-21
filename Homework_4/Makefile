##################################
# Makefile
# # Beckham Carver
# # COSC 3750, Spring 2024
# # Homework 4
# # 2/21/2023
# #
# # Makefile for HW4
# ##################################


CC=gcc
CFLAGS=-g -Wall -I.

# reference to clean and tidy recipes
.PHONY: clean tidy

# create approxe using approxe.c, compile with CC/CFLAGS + '-I'
wycat: wycat.c
        ${CC} ${CFLAGS} wycat.c -o wycat

# remove OBJS, and approxe
tidy:
	-rm -f wycat core
clean: tidy
