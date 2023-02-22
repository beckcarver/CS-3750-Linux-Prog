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
#include <strcmp.h>


// similar getline loop from man getline(3) but it never worked so I started
// fresh- and now they look nearly identical again
int intoOut(int buffsize) {
    char *buffer = malloc(buffsize);
    size_t len = buffsize;
    size_t read;
    
    while((read = getline(&buffer, &len, stdin)) != -1) {
        fwrite(buffer, sizeof(char), read, stdout);
    }
    
    free(buffer);
    return 1;
}

int main(int argc, char* argv[]) {
    const int BUFFSIZE = 4096;
    
    if(argc <= 1) {
        intoOut(BUFFSIZE);
     }
    
    for (int i = 1; i < argc; i++) {

        if(strcmp(argv[i], '-')) {
            intoOut(BUFFSIZE);
        }
        else if () {
            // read into buffer
        }

    }

}
