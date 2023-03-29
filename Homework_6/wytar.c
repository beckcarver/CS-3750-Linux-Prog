/* 
 * wytar.c
 * Author: Beckham Carver
 * Date: Mar 28, 2023
 * 
 * COSC 3750, Homework 6
 *
 * Modified version of 'tar' main code section
 * that parses arguments and passes them along 
 * for extraction or archival accordingly.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#include "wytar_utils.h"

void create_archive(char *archive_name, int num_files, char **file_names);
void extract_archive(char *archive_name);


int main(int argc, char *argv[]) {
    // Check if the user has provided enough arguments
    if (argc < 4) {
        fprintf(stderr, "Usage: %s -c|-x -f archive_name file1 [file2 ...]\n", argv[0]);
    	return -1;
    }

    // Check if -c or -x option is specified
    int create = 0;
    int extract = 0;
    if (strcmp(argv[1], "-c") == 0 && strcmp(argv[2], "-x") != 0) {
        perror("c found\n");
	create = 1;
    } else if (strcmp(argv[1], "-x") == 0 && strcmp(argv[2], "-c") != 0) {
        perror("x found\n");
	extract = 1;
    } else {
        perror("Error: You must specify either -c or -x option.\n");
    	return -1;
    }

    // Check if -f option is specified
    char* archive_name = NULL;
    int arg_index;
    for (arg_index = 2; arg_index < 6; arg_index++) {
        if (strcmp(argv[arg_index], "-f") == 0) {
            arg_index++;
            if (arg_index >= argc) {
                perror("Error: You must specify archive file name after -f option.\n");
            	return -1;
	    }
            archive_name = argv[arg_index];
            break;
        }
    }
    if (archive_name == NULL) {
        perror("Error: You must specify archive file name with -f option.\n");
    	exit(1);
    }

    // Check if there are files to be archived or extracted
    if (arg_index == argc) {
	perror("Error: You must specify at least one file to archive or extract.\n");
    	return -1;
    }

    // Create or extract the archive based on user's choice
    if (create) {
        create_archive(archive_name, arg_index, argv);
    } else if (extract) {
        extract_archive(archive_name);
    }


    return 0;
}
