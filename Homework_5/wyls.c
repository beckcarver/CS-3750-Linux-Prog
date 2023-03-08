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

#define PATH_MAX 4096
#define FNAME_MAX 256

// compiler complains about strchr getting a char**, but this is intended
// ncompatible-pointer-typesbecause argv[] is an array of char*, and we are pointing to the inner char*
// program fails without &


int process_dir(char* drname, int* flags);
void process_entry(char* fpath, char* fname, int* flags);
void print_perm(mode_t mode); 

int main(int argc, char* argv[]) { 

    // Initialize flags and end of options variables
    int flags[2] = {0};
    int pstop = 1;
    // searches for options until FIRST argument without '-' is found
    while(pstop < argc && argv[pstop][0] == '-') {
        pstop++;
        char opt[] = &argv[pstop][1];
        if(strchr(opt, 'n')) { 
            flags[0] = 1;
        }
        if(strchr(opt, 'h')) {
            flags[1] = 1;
        }
        printf(" %c %c\n", (char)flags[0], (char)flags[1]);
    }
    printf("pstop = %d\n", pstop);
    


    // if no additional arguments
    if (pstop >= argc) {
        process_dir(".", flags);
        return 0;
    }

    // starting from end of option arguments, checks if directory 
    // or a file and calls functions accordingly, until end of arguments
    int k = pstop;
    do {
        printf("remaining args, k = %d\n", k);
        char* fpath = realpath(argv[k], NULL);
        struct stat spath;

        // check if valid path at al
        if (stat(fpath, &spath) != 0) {  
            perror(strcat(argv[k], " invalid argument"));
        }
        // check if directory
        else if (S_ISDIR(spath.st_mode)) {
            process_dir(argv[k], flags); 
        }
        // not directory, and is a valid path
        else {
            process_entry(fpath, argv[k], flags);
        }
        
        k++;
    } while (k < argc);    

    
    return 0;
}


int process_dir(char *drname, int* flags) {
    DIR *dr;
    struct dirent *entry;
    dr = opendir(drname);
    if (dr == NULL) {
        perror(strcat(drname, " is not a valid directory"));
        return -1;
    }
    while((entry = readdir(dr)) != NULL) {
        char* fpath = realpath(entry->d_name, NULL);
        process_entry(fpath, entry->d_name, flags);
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

void process_entry(char* fpath, char* fname, int* flags) {
    // initialize stat, passwd, and group structires (not passed because
    // passing from directory function is unwieldy)
    struct stat spath;
    stat(fpath, &spath);
    struct passwd *pw = getpwuid(spath.st_uid);
    struct group *gr = getgrgid(spath.st_gid);


    // print permissions (always)
    print_perm(spath.st_mode);

    // print user/grp id's or names (flag dependent)
    if (flags[0] == 1) {
        printf(" %10d %10d", spath.st_uid, spath.st_gid);
    }
    else {
        printf(" %10s %10s", pw->pw_name, gr->gr_name);
    }

    // print size bytes or human readable (flag dependent)
    if (flags[1] == 1) {
        double size = (double)spath.st_size;
        if (size<1024){printf(" %10lf.0", size);}
        else if (size<1024*1024){printf(" %10f.1K", size/1024.0);}
        else if (size<1024*1024*1024){printf(" %10lf.1M", size/1024.0/1024.0);}
        else {printf(" %10lf.1G", size/1024.0/1024.0/1024.0);} 
    }
    else {
        printf(" %10ld", spath.st_size);
    }

    // print date (always)
    char *date_format = " %b %d %H:%M";
    if (time(NULL) - spath.st_mtime > 180 * 24 * 60 * 60) {
        date_format = " %b  %d %Y";
    }
    struct tm *tm = localtime(&spath.st_mtime);
    char date[64];
    strftime(date, sizeof(date), date_format, tm);
    printf(" %-s", date);

    // print filename given
    printf(" %-s\n", fname);
} 
