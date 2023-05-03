/*
 * wyargs.h
 * Author: Beckham Carver
 * Last Modified Date: May 2, 2023
 *
 * COSC 3750, Homework 10
 *
 * Header file for wyargs.c
 * 
 */

#ifndef WYARGS
#define WYARGS

typedef struct word Word;
typedef struct cmdLink CmdLink;

extern Word* createWord();
extern Word* appendWord(Word* word, const char* data);
extern void freeWords(Word* word);

extern CmdLink* createEmptyCmdLink();
extern CmdLink* appendCmdLink(CmdLink* cmdLink);
extern void freeCmdLinks(CmdLink* cmdLink);

#endif