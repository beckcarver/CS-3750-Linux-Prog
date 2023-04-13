/*
 * wytalkD.c
 * Author: Beckham Carver
 * Date April 11, 2023
 *
 * COSC 3750, Homework 7
 *
 * Server/Daemon side of 'Talk' utility, opens port 51100 and accepts connection
 * from clientside request, echoes input of client
 *
 */

#include "socketfun.h"
#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define PORT 51100


int readConnection(int connect);

int main(int argc, char *argv[]) {
	errno = 0;
	char hostname[256];

	if (gethostname(hostname, sizeof(hostname))) {
		perror("failed to get hostname");
		return -1;
	}

	printf("hostname is %s \n", hostname);
	
	int socket = serve_socket(hostname, PORT); 		
	if (socket < 3) {
		perror("serve_socket failed");
		return -1;
	}

	int connect = accept_connection(socket); 
	if (connect < 3) {
		perror("accept_connection failed");
		return -1;	
	}

	printf("connection established: host %s, socket %d, connection %d \n",
			hostname, socket, connect);
	
	char sendbuf[1024];
	readConnection(connect);

	while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
		if(write(connect, sendbuf, strlen(sendbuf)) == -1) {
			perror("error in sending/writing data");
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
			perror("zero bytes/error returns in read");
			return 1;
		}
		printf("%c", echo);
	}
	return 0;
}



