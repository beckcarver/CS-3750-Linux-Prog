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
    int input, output, error;
    char *infile, *outfile, *errfile;

    struct cmdLink *prev, *next;
};


Word* createWord(const char* data) {
    Word* newWord = (Word*)malloc(sizeof(Word));
    newWord->next = NULL;
    newWord->prev = NULL;
    newWord->data = strdup(data); // Copy the data string
    return newWord;
}

Word* appendWord(Word* word, const char* data) {
    Word* newWord = createWord(data);
    Word* currentWord = word;
    while(currentWord->next != NULL) {
        currentWord = currentWord->next;
    }
    currentWord->next = newWord;
    newWord->prev = currentWord;
    return newWord;
}

CmdLink* createCmdLink(const char* cmd_data) {
    Word* cmd_argv = createWord(cmd_data); // Create a new Word from cmd_data
    CmdLink* newCmdLink = (CmdLink*)malloc(sizeof(CmdLink));
    newCmdLink->cmd_argv = cmd_argv;
    newCmdLink->cmd_argc = 1;
    newCmdLink->input = 0;
    newCmdLink->output = 1;
    newCmdLink->error = 2;
    newCmdLink->infile = NULL;
    newCmdLink->outfile = NULL;
    newCmdLink->errfile = NULL;
    newCmdLink->prev = NULL;
    newCmdLink->next = NULL;
    return newCmdLink;
}

CmdLink* appendCmdLink(CmdLink* current, const char* cmd_data) {
    CmdLink* newCmdLink = createCmdLink(cmd_data);
    current->next = newCmdLink;
    newCmdLink->prev = current;
    return newCmdLink;
}


void freeWords(Word* word) {
    while (word != NULL) {
        Word* temp = word;
        word = word->next;
        free(temp->data);
        free(temp);
    }
}

void freeCmdLinks(CmdLink* cmdLink) {
    while (cmdLink != NULL) {
        CmdLink* tmp = cmdLink;
        cmdLink = cmdLink->prev;
        freeWords(tmp->cmd_argv);
        free(tmp->cmd_argv);
        free(tmp->infile);
        free(tmp->outfile);
        free(tmp->errfile);
        free(tmp);
    }
}
