/*
Name: Aviv Zvuluny
ID: 211313333

String Parser - Ex 1:

This program gets user input over and over until the word "exit" is received.
The program counts how many words were entered and how many characters are in each word (excluding spaces)
Finally the program saves the words entered to a file called history, and loads them from the same file if the user requested by entering "history"

key words:
    exit
    history

input from the user must not exceed "MAX_LINE_LENGTH" characters
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#define MAX_LINE_LENGTH 510         // the max length of the input string from the user (excluding '\0')
#define EXPAND_LIST_MULTIPLIER 1.3  // arbitrary 1.3, nothing special about it, it just felt right (warning: don't go below 1)

void loadHistory(char*, char***);
void saveHistory(char*, char** const);
void printHistory(char**);
void expandList(char***, int, int, double);
void deallocateMemory(char**, int);
void analyzeLine(char*, char***);

int historyCapacity = 0; // capacity is the number of elements in historyList
int historySize = 10;   // starting with an arbitrary size of 10, will expand if necessary

/**
 * main loop.
 * Each time asks for an input from the user, analyzes the line, saves history to fileName and repeats.
 * if the word "history" is entered, load all the history from fileName to historyList and print it to the user.
*/
int main() {
    char* fileName = "history";
    char** historyList = (char**)malloc(sizeof(char*) * historySize);
    if (historyList == NULL) {
        printf("malloc ailed for historyList in main()\n");
    }
    loadHistory(fileName, &historyList);

    //get first input
    printf("Enter String, or \"exit\" to end program:\n");
    char line[MAX_LINE_LENGTH + 1];
    fgets(line, MAX_LINE_LENGTH + 1, stdin);
    // in case of excess input (line is longer than MAX_LINE_LENGTH), clean the buffer
    fflush(stdin);
    line[strlen(line) - 1] = '\0';
    //main loop
    while (strcmp(line, "exit") != 0) {
        if (strcmp(line, "history") == 0) {
            loadHistory(fileName, &historyList);
            printHistory(historyList);

            printf("Enter String, or \"exit\" to end program:\n");
            fgets(line, MAX_LINE_LENGTH + 1, stdin);
            fflush(stdin);
            line[strlen(line) - 1] = '\0';
            continue;
        }
        //analyze words (print them and count words) and then save to the history file
        analyzeLine(line, &historyList);
        saveHistory(fileName, historyList);

        printf("Enter String, or \"exit\" to end program:\n");
        fgets(line, MAX_LINE_LENGTH + 1, stdin);
        fflush(stdin);
        line[strcspn(line, "\n")] = '\0';
    }
    //free memory
    deallocateMemory(historyList, historyCapacity);
    return 0;
}

/**
 * char * line: the input sentence from the user, max of MAX_LINE_LENGTH characters
 * char *** historyList a pointer to historyList, since we will be making changes to it (adding new strings)
 *
 * analyzeLine scans line for words and counts characters, then prints the word to the screen, the word count and the character count.
 * then, saves the words to historyList and expands historyList if necessary
 *
 * usage example: analyzeLine("asd", &historyList);
*/
void analyzeLine(char* line, char*** historyList) {
    int wordCount = 0;
    int charCount = 0;
    int startingIndex = 0;
    char toSaveInHistory[MAX_LINE_LENGTH + 1]; // ultimately the string that will be saved in historyList
    toSaveInHistory[0] = '\0';

    //find a word (words are separated by spaces), allocate memory for it, print info to the user, free memory allocated to word
    for (int i = 0; i < strlen(line) + 1; i++) {
        if (line[i] == ' ' || line[i] == '\0') {
            //if spaces at the start, skip them
            if (i == 0) {
                //skip all spaces
                while (i < strlen(line) - 1 && line[i + 1] == ' ') {
                    i++;
                }
                startingIndex = i + 1;
                continue;
            }
            //allocate memory for word, save it in toSaveInHistory and free word
            wordCount++;
            char* word = (char*)malloc(sizeof(char) * (i - startingIndex + 1));
            if (word == NULL) {
                printf("malloc failed for char* word in analyzeLine()\n");
                exit(1);
            }
            strncpy(word, line + startingIndex, i - startingIndex);
            word[i - startingIndex] = '\0';
            strcat(toSaveInHistory, word);
            if (line[i] == ' ') {
                strcat(toSaveInHistory, " ");
            }
            charCount += strlen(word);
            printf("%s ", word);
            free(word);
            //skip all spaces
            while (i < strlen(line) - 1 && line[i + 1] == ' ') {
                i++;
            }
            startingIndex = i + 1;
        }
    }
    printf("\n%d words\n", wordCount);
    printf("%d chars\n", charCount);

    //save toSaveInHistory to historyList. if we lack space, expand historyList and add after expanding.
    if (historyCapacity >= historySize) {
        expandList(historyList, historySize, historyCapacity, EXPAND_LIST_MULTIPLIER);
    }
    (*(historyList))[historyCapacity] = (char*)malloc(sizeof(char) * (strlen(toSaveInHistory) + 1));
    if ((*(historyList))[historyCapacity] == NULL) {
        printf("malloc failed for (*(historyList))[historyCapacity]\n");
        exit(1);
    }
    strcpy((*historyList)[historyCapacity], toSaveInHistory);
    historyCapacity++;
}

/**
 * prints historyList in this format:
 * i: <sentence>
 */
void printHistory(char** historyList) {
    for (int i = 0; i < historyCapacity; i++) {
        printf("%d: %s\n", i, historyList[i]);
    }
    printf("\n");
}

/**
 * frees all the memory pointed by lst, and the memory allocated for lst
 * int size - size of the list pointed at by lst
 *
 * usage example: deallocateMemory(historyList, 10); where 10 is the number elements in the list (not the size)
 */
void deallocateMemory(char** lst, int size) {
    for (int i = 0; i < size; i++) {
        free(lst[i]);
    }
    free(lst);
}

/**
 * char *** lst a pointer to the list to expand, this list will be changed to be the new and bigger list.
 * int currentSize - size before expanding.
 * int capacity - how many slots are filled (have memory allocated to them).
 * double sizeMultiplier - how much to multiply lst's size by.
 *
 * This method allocates more memory for lst to use, used mostly when capacity == currentSize
 *
 * usage example: expandList(historyList, historySize, historyCapcity, strlen(line), 2.4);
*/
void expandList(char*** lst, int currentSize, int capacity, double sizeMultiplier) {
    char** temp = *lst;
    int newSize = currentSize * sizeMultiplier + 1;
    //allocate new memory for lst, delete old lst later
    *lst = (char**)malloc(sizeof(char*) * newSize);
    if (*lst == NULL) {
        printf("malloc failed for *lst in expandList\n");
        exit(1);
    }
    //copy elements of old lst to new lst
    for (int i = 0; i < capacity; i++) {
        (*lst)[i] = (char*)malloc(sizeof(char) * (strlen(temp[i]) + 1));
        if ((*lst)[i] == NULL) {
            printf("malloc failed for *lst[i] in expandList\n");
            exit(1);
        }
        strcpy((*lst)[i], temp[i]);
    }
    historySize = newSize;
    // delete old lst
    deallocateMemory(temp, historyCapacity);
}

/**
 * char * fileName - file to gather information from
 * char *** historyList pointer to historyList, since we will be changing it in case capacity == size
 *
 * loads the lines written in fileName to historyList, excluding the new line character
 * strings aren't stored in any format, every line is loaded into a different cell of historyList
 *
 * usage example: loadHistory("file_name", &historyList);
*/
void loadHistory(char* fileName, char*** historyList) {
    FILE* historyFile;
    // if file doesn't exist, create it
    if (access(fileName, F_OK) == 0) {
        historyFile = fopen(fileName, "r");
        if (historyFile == NULL) {
            printf("error while opening file\n");
            exit(1);
        }
    }
    else {
        FILE* temp = fopen(fileName, "w");
        fclose(temp);
        historyFile = fopen(fileName, "r");
    }

    // read lines until end of file and put them into historyList
    char line[MAX_LINE_LENGTH + 1];
    int lineLength;
    int i = 0;
    while (fgets(line, MAX_LINE_LENGTH, historyFile) != NULL) {
        if (i >= historySize) {
            expandList(historyList, historySize, historyCapacity, EXPAND_LIST_MULTIPLIER);
        }
        line[strlen(line) - 1] = '\0';
        //if we called this function before, fill only the slots that have memory allocated to them
        if (i >= historyCapacity) {
            (*historyList)[i] = (char*)malloc(sizeof(char) * (strlen(line) + 1));
            if ((*historyList)[i] == NULL) {
                printf("malloc failed for (*historyList)[i] in loadHistory()\n");
                fclose(historyFile);
                exit(1);
            }
            historyCapacity++;
            strcpy((*historyList)[i], line);
        }
        else {
            strcpy((*historyList)[i], line);
        }
        i++;
    }
    fclose(historyFile);
}

/**
 * char * fileName - file to save history to
 * char ** const historyList - list of strings to save to the file. const since we won't be changing it.
 *
 * saves all the strings stored in historyList to the file fileName
 * there is no format for the strings stored, each string is stored in a different line.
 *
 * usage example: saveHistory("file_name", historyList);
*/
void saveHistory(char* fileName, char** const historyList) {
    FILE* historyFile = fopen(fileName, "w");
    if (historyFile == NULL) {
        printf("error while opening file\n");
        exit(1);
    }
    //loop over all the elements in historyList and add them to each line of historyFile
    char line[MAX_LINE_LENGTH + 1];
    for (int i = 0; i < historyCapacity; i++) {
        char* temp = (char*)malloc(sizeof(char) * (strlen(historyList[i]) + 2));
        if (temp == NULL) {
            printf("malloc failed for char* temp in saveHistory()\n");
            fclose(historyFile);
            exit(1);
        }
        strcpy(temp, historyList[i]);
        strcat(temp, "\n");
        fputs(temp, historyFile);
        free(temp);
    }
    fclose(historyFile);
}