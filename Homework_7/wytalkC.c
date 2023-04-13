/*
 * wytalkC.c
 * Author: Beckham Carver
 * Date: April 11, 2023
 *
 * COSC 3750, Homework 7
 *
 * Client side of 'Talk' utility on port 51100 that simply echos input of one
 * machine to other
 */

#include "socketfun.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define PORT 51100


int readConnection(int connect);

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("incorrect number of arguments");
		return -1;
	}

	int connect = request_connection(argv[1], PORT);
	if (connect < 3) {
		printf("connection failed");
		return -1;
	}
	
	char sendbuf[1024];
	
	while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
		if(write(connect, sendbuf, strlen(sendbuf)) == -1) {
			perror("failed to write/send");
			break;
		}
		if (readConnection(connect)) {break;}
	}
	
	close(connect);

	return 0;
}

int readConnection(int connect) {
	char echo;
	while(echo != '\n') {
		// if zero bytes read, or error (-1) is returned
		if (read(connect, &echo, 1) <= 0) {	
			perror("zero/error bytes read");
			return 1;
		}
		printf("%c", echo);
	}
	return 0;
}
