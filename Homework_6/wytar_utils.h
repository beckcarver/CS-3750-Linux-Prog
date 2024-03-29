/* 
 * wytar_utils.h
 * Author: Beckham Carver
 * Date: Mar 28, 2023
 * 
 * COSC 3750, Homework 6
 *
 * Modified version of 'tar' utility,
 * holds struct definition for a tar_header
 * and template funtions used in wytar_utils.c
 *
 */

#include <tar.h>


struct tar_header
{ /* byte offset */
	char name[100]; /* 0 */
	char mode[8]; /* 100 */
	char uid[8]; /* 108 */
	char gid[8]; /* 116 */
	char size[12]; /* 124 */
	char mtime[12]; /* 136 */
	char chksum[8]; /* 148 */
	char typeflag; /* 156 */
	char linkname[100]; /* 157 */
	char magic[6]; /* 257 */
	char version[2]; /* 263 */
	char uname[32]; /* 265 */
	char gname[32]; /* 297 */
	char devmajor[8]; /* 329 */
	char devminor[8]; /* 337 */
	char prefix[155]; /* 345 */
	char pad[12]; /* 500 */
};


void create_archive(char *archive_name, int num_files, char **file_names);
void extract_archive(char *archive_name);
