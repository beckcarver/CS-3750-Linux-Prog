/* 
 * wytar_util.c
 * Author: Beckham Carver
 * Date: Mar 28, 2023
 * 
 * COSC 3750, Homework 6
 *
 * Modified version of 'tar' utility
 * core logic of the creation and extraction
 * of tar files. These functions are called
 * from wytar.c
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "wytar_utils.h"

#define BLOCK_SIZE 512


void extract_archive(char *archive_name) {

    FILE *archive_file = fopen(archive_name, "rb");
    if (archive_file == NULL) {
	perror("Error opening archive file (23)");
    	fprintf(stderr, "given archive name is: %s \n", archive_name);
	exit(EXIT_FAILURE);
    }

    struct tar_header header;
    int read_size = fread(&header, sizeof(struct tar_header), 1, archive_file);
    while (read_size == 1 && header.name[0] != '\0') {
        char *filename = header.name;
        mode_t mode = strtol(header.mode, NULL, 8);
	fprintf(stderr, "mode number is %d \nmode string is %s \n", mode, header.mode);
        if (S_ISDIR(mode) || mode == 493) {
            if (mkdir(filename, mode) != 0 && errno != EEXIST) {
                perror("Error creating directory (36)");
                exit(EXIT_FAILURE);
            }
            printf("Directory created: %s\n", filename);
        } else if (S_ISLNK(mode)) {
            if (symlink(header.linkname, filename) != 0) {
                perror("Error creating symlink (42)");
                exit(EXIT_FAILURE);
            }
            printf("Symlink created: %s -> %s\n", filename, header.linkname);
        } else if (mode > 0) {
            FILE *file = fopen(filename, "wb");
            if (file == NULL) {
                // Try creating any non-existent directories in the file path
                char *last_slash = strrchr(filename, '/');
                if (last_slash != NULL) {
                    *last_slash = '\0';
                    if (mkdir(filename, S_IRWXU) != 0 && errno != EEXIST) {
                        perror("Error creating directory (54)");
                        exit(EXIT_FAILURE);
                    }
                    *last_slash = '/';
                }

                // Try opening file again
                file = fopen(filename, "wb");
                if (file == NULL) {
                    perror("Error opening file for writing (63)");
                    exit(EXIT_FAILURE);
                }
            }
	    
            size_t file_size = strtol(header.size, NULL, 8);
            size_t num_blocks = (file_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
            for (size_t i = 0; i < num_blocks; i++) {
                char buffer[BLOCK_SIZE];
                size_t read_bytes = fread(buffer, 1, BLOCK_SIZE, archive_file);
                if (read_bytes < BLOCK_SIZE && ferror(archive_file)) {
                    perror("Error reading from archive file (74)");
                    exit(EXIT_FAILURE);
                }
                size_t write_bytes = fwrite(buffer, 1, read_bytes, file);
                if (write_bytes < read_bytes) {
                    perror("Error writing to file (79)");
                    exit(EXIT_FAILURE);
                }
            }

            if (fclose(file) != 0) {
                perror("Error closing file");
                exit(EXIT_FAILURE);
            }
            printf("File extracted: %s\n", filename);
        } else {
            printf("Unsupported file type: %s (90) \n", filename);
        }

        // Read next header
        read_size = fread(&header, sizeof(struct tar_header), 1, archive_file);
    }

    if (read_size < 1) {
        perror("Error reading from archive file (98)");
        exit(EXIT_FAILURE);
    }

    if (fclose(archive_file) != 0) {
        perror("Error closing archive file (103)");
        exit(EXIT_FAILURE);
    }
}





void create_archive(char *archive_name, int start_ind, char **file_names) {
    // Open archive file for writing
    FILE *archive_file = fopen(archive_name, "w");
    if (archive_file == NULL) {
        perror("Error opening archive file");
        exit(EXIT_FAILURE);
    }

    // Write headers and file contents to archive
    int num_files = sizeof(file_names);
    int i = start_ind;
    for (; i < num_files; i++) {
        // Get file stats
        struct stat file_stats;
        if (stat(file_names[i], &file_stats) == -1) {
            perror(file_names[i]);
            continue;
        }

        // Open file for reading
        FILE *file = fopen(file_names[i], "r");
        if (file == NULL) {
            perror(file_names[i]);
            continue;
        }

        // Write header to archive
        struct tar_header header;
        memset(&header, 0, sizeof(header));
        strcpy(header.name, file_names[i]);
        sprintf(header.mode, "%07o", file_stats.st_mode & 0777);
        sprintf(header.uid, "%07o", file_stats.st_uid);
        sprintf(header.gid, "%07o", file_stats.st_gid);
        sprintf(header.size, "%011lo", file_stats.st_size);
        sprintf(header.mtime, "%011lo", file_stats.st_mtime);

	if(S_ISDIR(file_stats.st_mode) != 0){
	    // directory
	    header.typeflag = DIRTYPE;
	} else if(S_ISLNK(file_stats.st_mode) != 0) {
            // link so we dont put a size just leave as zeros
	    header.typeflag = SYMTYPE;
       	    strcpy( header.size,  "00000000000");
       	    readlink(file_names[i], header.linkname, 100);
    	} else {
            // not a dir or link
       	    header.typeflag = REGTYPE;
    	}

        strcpy(header.magic, "ustar");
        strcpy(header.version, "00");
        strcpy(header.uname, "user");
        strcpy(header.gname, "user");
        fwrite(&header, sizeof(header), 1, archive_file);

        // Write file contents to archive
        int remaining_bytes = file_stats.st_size;
        while (remaining_bytes > 0) {
            char buffer[BLOCK_SIZE];
            int bytes_read = fread(buffer, 1, BLOCK_SIZE, file);
            if (bytes_read == 0 && ferror(file)) {
                perror(file_names[i]);
                fclose(file);
                fclose(archive_file);
                exit(EXIT_FAILURE);
            }
            fwrite(buffer, 1, bytes_read, archive_file);
            remaining_bytes -= bytes_read;
        }

        // Pad the last block with zeros, if necessary
        int padding_bytes = BLOCK_SIZE - (file_stats.st_size % BLOCK_SIZE);
        if (padding_bytes == BLOCK_SIZE) {
            padding_bytes = 0;
        }
        char padding_buffer[BLOCK_SIZE];
        memset(padding_buffer, 0, padding_bytes);
        fwrite(padding_buffer, 1, padding_bytes, archive_file);

        fclose(file);
    }

    // Write two empty blocks at the end of the archive
    char zero_block[BLOCK_SIZE];
    memset(zero_block, 0, BLOCK_SIZE);
    fwrite(zero_block, 1, BLOCK_SIZE, archive_file);
    fwrite(zero_block, 1, BLOCK_SIZE, archive_file);

    fclose(archive_file);
}


