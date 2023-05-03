/*
 * wyargs.c
 * Author: Beckham Carver
 * Date: April 25, 2023
 *
 * COSC 3750, Homework 9
 *
 * Data structure definition for command linker, and 'Word' linked list
 * as well as helper functions regarding those structures
 * 
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


typedef struct word Word;
typedef struct cmdLink CmdLink;

struct word {
    struct word *next, *prev;
    char* data;
};

struct cmdLink {
    Word *cmd_argv;
    int cmd_argc;

    // set depending on pipe
    int input, output, error, amp;
    char *infile, *outfile, *errfile;

    struct cmdLink *prev, *next;
};


Word* createEmptyWord() {
    Word* newWord = (Word*)malloc(sizeof(Word));
    newWord->data = NULL;
    newWord->next = NULL;
    newWord->prev = NULL;
    return newWord;
}

Word* appendWord(Word* word, const char* data) {
    Word* tmp = word;
    while(tmp->next != NULL) {
        tmp = tmp->next;
    }
    tmp->data = strdup(data);
    Word* newWord = createEmptyWord();
    tmp->next = newWord;
    newWord->prev = tmp;
    return newWord;
}

CmdLink* createEmptyCmdLink() {
    Word* cmd_argv = createEmptyWord(); // Create a new Word from cmd_data
    CmdLink* newCmdLink = (CmdLink*)malloc(sizeof(CmdLink));
    newCmdLink->cmd_argv = cmd_argv;
    newCmdLink->cmd_argc = 0;
    newCmdLink->input = 0;
    newCmdLink->output = 1;
    newCmdLink->error = 2;
    newCmdLink->amp = 0;
    newCmdLink->infile = NULL;
    newCmdLink->outfile = NULL;
    newCmdLink->errfile = NULL;
    newCmdLink->prev = NULL;
    newCmdLink->next = NULL;
    return newCmdLink;
}

CmdLink* appendCmdLink(CmdLink* current) {
    CmdLink* newCmdLink = createEmptyCmdLink();
    current->next = newCmdLink;
    newCmdLink->prev = current;
    return newCmdLink;
}


void freeWords(Word* word) {
    if (word == NULL) {
        return;
    }
    //printf("----free WORD\n");
    freeWords(word->next);
    free(word->data);   
    free(word); 
}

void freeCmdLinks(CmdLink* cmdLink) {
    if (cmdLink == NULL) {
        return;
    }
    //printf("free LINK\n");
    freeWords(cmdLink->cmd_argv);
    freeCmdLinks(cmdLink->next);
    free(cmdLink->infile);
    free(cmdLink->outfile);
    free(cmdLink->errfile);
    free(cmdLink);
}


    
