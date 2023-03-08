/* 
 * wyls.c
 * Author: Beckham Carver
 * Date: Mar 7, 2023
 * 
 * COSC 3750, Homework 5
 *
 * Modified version of 'ls' utility
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <regex.h>
#include <time.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

// compiler complains about strchr getting a char**, but this is intended
// ncompatible-pointer-typesbecause argv[] is an array of char*, and we are pointing to the inner char*
// program fails without &


int process_dir(char* drname, int* flags);
void process_entry(char *file, int* flags);
void print_perm(mode_t mode); 

int main(int argc, char* argv[]) { 

    // Initialize flags and end of options variables
    int flags[2] = {0};
    int pstop = 1;
    // searches for options until FIRST argument without '-' is found
    while(pstop < argc && argv[pstop][0] == '-') {
        pstop++;
        if(strchr(&argv[pstop], 'n')) { 
            flags[0] = 1;
        }
        if(strchr(&argv[pstop], 'h')) {
            flags[1] = 1;
        }
    }
    printf("pstop = %d\n", pstop);
    


    // if no additional arguments
    if (pstop >= argc) {
        procDir(".", flags);
        return 0;
    }


    // starting from end of option arguments, checks if directory 
    // or a file and calls functions accordingly, until end of arguments
    int k = pstop;
    do {
        printf("remaining args, k = %d\n", k);
        if (
        
        
        k++;
    } while (k < argc);    

    
    return 0;
}


int process_dir(char* drname, int* flags) {
    DIR *dr;
    struct dirent *entry;
    dr = opendir(drname);
    if (dr == NULL) {
        perror(strcat(drname, " is not a valid directory"));
        return -1;
    }
    while((entry = readdir(dr)) != NULL) {
        process_entry(entry->dnname, flags);
    }
    return 0;
}

void print_perm(mode_t mode) {
    printf((S_ISDIR(mode)) ? "d" : ((S_ISLNK(mode)) ? "l" : "-"));
    printf((mode & S_IRUSR) ? "r" : "-");
    printf((mode & S_IWUSR) ? "w" : "-");
    printf((mode & S_IXUSR) ? "x" : "-");
    printf((mode & S_IRGRP) ? "r" : "-");
    printf((mode & S_IWGRP) ? "w" : "-");
    printf((mode & S_IXGRP) ? "x" : "-");
    printf((mode & S_IROTH) ? "r" : "-");
    printf((mode & S_IWOTH) ? "w" : "-");
    printf((mode & S_IXOTH) ? "x" : "-");
}
