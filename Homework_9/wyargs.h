/*
 * wyargs.h
 * Author: Beckham Carver
 * Date: April 25, 2023
 *
 * COSC 3750, Homework 9
 *
 * Header file for wyargs.c
 * 
 */

#ifndef WYARGS
#define WYARGS

typedef struct word Word;
typedef struct cmdLink CmdLink;

extern Word* createWord(const char* data);
extern Word* appendWord(Word* word, const char* data);
extern void freeWords(Word* word);

extern CmdLink* createCmdLink(const char* cmd_data);
extern CmdLink* appendCmdLink(CmdLink* cmdLink, const char* cmd_data);
extern void freeCmdLinks(CmdLink* cmdLink);

#endif