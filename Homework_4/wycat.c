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


// similar getline loop from man getline(3) but it never worked so I started
// fresh- and now they look nearly identical again
int intoOut(int buffsize);

int main(int argc, char* argv[]) {
    const int BUFFSIZE = 4096;
    if(argc <= 1) {
        intoOut(BUFFSIZE);
    }
    else {
        
        // debug
        for (int i = 1; i < argc; i++) {
            printf(argv[i]);
            printf("  ");
        }
        printf("\n end of args \n");


        for (int i = 1; i < argc; i++) {
         
            FILE *stream = fopen(argv[i], "r");
            
            if(!strcmp(argv[i], "-")) {
                intoOut(BUFFSIZE);
            }
            else if (stream != NULL) {
                // file is readable
                printf("\n reading file \n");
                char *buffer = malloc(BUFFSIZE);
                size_t read = fread(buffer, sizeof(char), BUFFSIZE, stream);
                fwrite(buffer, sizeof(char), read, stdout);
                free(buffer);
                fclose(stream);
            }
            else {
                // file unreadable
                char *errInput = argv[i];
                perror(errInput);
            }        
        }
    }
    return 0;
}

int intoOut(int buffsize) {
    char *buffer = malloc(buffsize);
    size_t len = buffsize;
    size_t read;
    
    while((read = getline(&buffer, &len, stdin)) != -1) {
        fwrite(buffer, sizeof(char), read, stdout);
    }
    
    free(buffer);
    return 0;
}


