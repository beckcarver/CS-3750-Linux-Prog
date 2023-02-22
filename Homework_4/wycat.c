/* 
 * wycat.c
 * Author: Beckham Carver
 * Date: Feb 21, 2023
 * 
 * COSC 3750, Homework 4
 *
 * Modified version of 'cat' utility
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


void intoOut(int buffsize);

int main(int argc, char* argv[]) {
    
    const int BUFFSIZE = 4096;
    
    if(argc <= 1) {
        intoOut(BUFFSIZE);
        return 0;
    }
    
    for (int i = 1; i < argc; i++) {
     
        FILE *stream;
        
        if(!strcmp(argv[i], "-")) {
            // dash in arguments
            intoOut(BUFFSIZE);
        }
        else if ((stream = fopen(argv[i], "r")) != NULL) {
            // file is readable
            char *buffer = malloc(BUFFSIZE);
            size_t read;
            while((read = fread(buffer, sizeof(char), BUFFSIZE, stream)) != 0){
                int written = fwrite(buffer, sizeof(char), read, stdout);
                if (written != read) {
                    perror(strcat("different vals read/written from ", argv[i]));
                    break;
                }
            }    
            free(buffer);
            fclose(stream);
        }
        else {
            // file unreadable
            char *errInput = argv[i];
            perror(errInput);
        }        
    }
    return 0;
}


void intoOut(int buffsize) {
    char *buffer = malloc(buffsize);
    size_t read;
    size_t len;
   
    // this took my so long to figure out, without it, one use of cntl+d means
    // all subsequent readings of stdin terminate early, ie. no multiple -`s
    clearerr(stdin);
    
    while((read = getline(&buffer, &len, stdin)) != -1) {
        int written = fwrite(buffer, sizeof(char), read, stdout);
        if (written != read) {
            perror("different vals read/written from stdin");
        }
    }

    free(buffer);
}


