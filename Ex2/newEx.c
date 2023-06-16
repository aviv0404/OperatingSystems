/*
Name: Aviv Zvuluny
ID: 211313333

Shell Simulator - Ex 2:
This program gets user input over and over until the word "done" is received.
The program counts how many commands were entered and how many words are in each command (excluding spaces)
The program then executes the user's commands and finally, saves the words entered to a file called history,
and loads them from the same file if the user requested by entering "history"

key words:
    done
    history
    !<previousCommandIndex>

input from the user must not exceed "MAX_LINE_LENGTH" characters
*/
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_LINE_LENGTH 512         // the max length of the input string from the user (excluding '\0')
#define EXPAND_LIST_MULTIPLIER 1.3  // arbitrary 1.3, nothing special about it, it just felt right (warning: don't go below 1)

//functions
void loadHistory(char*, char***);
void saveHistory(char*, char** );
void printHistory(char**);
void expandList(char***, int, int, double);
void deallocateMemory(char**, int);
void analyzeLine(char*, char***);
void getCommands();
void executeCommandHelper(int, char*);
void executeCommand(int, char**);
void executePipeCommands(int , char **, int , char **, int );
void getInput(char * , char **);
int isEmpty(char*);
int countCharInString(char, char *);
char* removeSpaces(char*);

int totalCommands = 0; // the number of words in all commands
int totalNumOfWordsInCommands = 0;  // the number of total commands entered

int totalPipes = 0; // total number of pipes across the life of the program

int historyCapacity = 0; // capacity is the number of elements in historyList
int historySize = 10;   // starting with an arbitrary size of 10, will expand if necessary
char* fileName = "history"; // history file name
char path[MAX_LINE_LENGTH]; // full path to current folder

/**
 * start of process, calls getCommands where the main loop starts.
 * at the end prints statistics about the user's commands.
 */
int main() {
    getCommands();
    //print statistics
    printf("Num of commands: %d\n", totalCommands);
    printf("Total number of words in all commands: %d !\n\n", totalNumOfWordsInCommands);
    return 0;
}

/**
 * @brief uses fork + execvp to execute the input command in argv
 *
 * @param argc arguments count
 * @param argv arguments values
 */
void executeCommand(int argc, char* argv[]) {
    //gather statistics
    totalCommands++;
    totalNumOfWordsInCommands += argc;

    char path[MAX_LINE_LENGTH];

    pid_t child = fork();
    if (child == 0) {
        //check for "cd" command
        if (strcmp(argv[0], "cd") == 0) {
            printf("command not supported (Yet)\n");
        }
        else {
            int a = execvp(argv[0], argv);
            printf(a);
        }
        exit(1);
    }
    else if (child == -1) {
        perror("\nfork error");
    }
    else {
        //parent process, wait for child to execute command
        wait(0);
    }
}

/**
 * main loop.
 * Each time asks for an input from the user, analyzes the line/command, executes user command, saves history of commands
 * to fileName and repeats.
 * if the word "history" is entered, loads all the history from fileName to historyList and print it to the user.
 * if the word "done" is entered, exits process and frees all allocated memory
*/
void getCommands() {
    int argc;
    char** historyList = (char**)malloc(sizeof(char*) * historySize);
    if (historyList == NULL) {
        fprintf(stderr,"malloc failed for historyList in main()\n");
    }
    loadHistory(fileName, &historyList);

    //get first input
    printf("%s>",getcwd(path, sizeof(path)));
    char argv[MAX_LINE_LENGTH + 1];
    fgets(argv, MAX_LINE_LENGTH + 1, stdin);
    fflush(stdin);// in case of excess input (line is longer than MAX_LINE_LENGTH), clean the buffer
    argv[strlen(argv) - 1] = '\0';
    char* argvNoSpaces = removeSpaces(argv);
    //main loop
    while (strcmp(argvNoSpaces, "done") != 0 || strstr(argv,"done") == NULL) {
        //if input is whitespace or \0 alon e, get a new input
        if (isEmpty(argv) == 0){
            getInput(argv, &argvNoSpaces);
            continue;
        }
        //count pipes, then analyze the string based on the amount of pipes
        int numOfPipes = countCharInString('|', argv);
        if (numOfPipes == 2){

        }else if (numOfPipes == 1){
            char * temp = strstr(argv, "|");
            int pipeIndex = (int)(temp - argv);
            // generate left
            char * left = (char *) malloc(sizeof(char) * (pipeIndex + 1));
            left[pipeIndex + 1] = '\0';
            strncpy(left, argv, pipeIndex);
            // generate right
            char * right = (char *) malloc(sizeof(char) * (strlen(argv) - pipeIndex));
            right[strlen(argv) - pipeIndex] = '\0';
            strncpy(right, temp+1, strlen(argv) - pipeIndex);
            // analyze
            
            //test
            char * argvLeft[4];
            argvLeft[0] = "echo";
            argvLeft[1] = "abc";
            argvLeft[2] = "qwe";
            argvLeft[3] = NULL;

            char** argvRight = (char**) malloc(sizeof(char*) * 4);
            for (int i = 0; i < 4; i++) {
                argvRight[i] = (char*) malloc(sizeof(char) * 10);
            }
            
            strcpy(argvRight[0], "wc");
            strcpy(argvRight[1], "-l");
            executePipeCommands(4, argvLeft, 4, argvRight, 4);

            // execute, pipe

            //free memory
            free(left);
            free(right);
        }else{
            //analyze words (print them and count words) and then save to the history file
            analyzeLine(argv, &historyList);
            saveHistory(fileName, historyList);
        }
        //get input
        getInput(argv, &argvNoSpaces);
    }
    //free memory
    free(argvNoSpaces);
    deallocateMemory(historyList, historyCapacity);
}

//TODO: comment
int countCharInString(char ch, char * str){
    int result = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == ch){
            result++;
        }
    }
    return result;
}

//TODO: comment
void getInput(char * line, char ** argvNoSpaces) {
    printf("%s> ",getcwd(path, sizeof(path)));
    fgets(line, MAX_LINE_LENGTH + 1, stdin);
    fflush(stdin);
    line[strcspn(line, "\n")] = '\0';
    free(*argvNoSpaces);
    *argvNoSpaces = removeSpaces(line);
}

/**
 * @brief returns 0 if str is empty and 1 otherwise. helper function for getCommands()
 * a string will be considered empty if it contains only whitespaces and '\0'
 */
int isEmpty(char* str) {
    if (str[0] == '\0') {
        return 0;
    }
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] != ' ' && str[i] != '\0' && str[i] != "\n") {
            return 1;;
        }
        i++;
    }
    return 0;
}

//TODO: comment
void executePipeCommands(int argcLeft, char * argvLeft[], int argcRight, char * argvRight[], int argvRightLen){
    //check for "cd" command
    if (strcmp(argvLeft[0], "cd") == 0 || strcmp(argvRight[0], "cd") == 0) {
            printf("command not supported (Yet)\n");
            return;
    }
    
    //create pipe
    int pfds[2];
    int temp = pipe(pfds);
    if (temp == -1){
        perror("error creating pipe\n");
        exit(1);
    }
    // fork left child (left child will execute argvLeft)
    pid_t leftChild = fork();
    if (leftChild == 0) {
        //left child process
        //redirect stdout into pfds[1] (write) and close read.
        dup2(pfds[1], STDOUT_FILENO);
        close(pfds[0]);
        close(pfds[1]);

        int execStatus = execvp(argvLeft[0], argvLeft);
        if (execStatus == -1){
            fprintf(stderr,"Command not supported\n");
            exit(1);
        }
        exit(1);
    }
    else if (leftChild == -1) {
        perror("fork error\n");
    }
    else {
        //parent process, wait for left child to execute command, close pipes
        close(pfds[0]);
        close(pfds[1]);

        int leftStatus;
        wait(leftChild,&leftStatus, -1); // wait for left child
        if (WEXITSTATUS(leftStatus) == 1){
            return;
        }

        int rightChild = fork();
        if (rightChild == 0){
            //right child process
            //redirect stdin into pfds[0] (read) and close write
            dup2(pfds[0], STDOUT_FILENO);
            close(pfds[1]);

            //TODO: FIX THIS NO WORK
            char leftResult[MAX_LINE_LENGTH];
            scanf("%s", leftResult);
            close(pfds[0]);
            //insert left result into argvRight
            argvRight[argvRightLen - 2] = (char *)malloc(sizeof(char) * (strlen(leftResult) + 1));
            strcpy(argvRight[argvRightLen - 2], leftResult);
            argvRight[argvRightLen - 1] = NULL;

            //execute right
            int execStatus = execvp(argvRight[0],argvRight);
            if (execStatus == -1){
                fprintf(stderr, "Command not supported\n");
                exit(1);
            }
        }else if(rightChild == -1){
            perror("\nfork error\n");
        }else{
            //parent process, wait for right child to execute command

            int rightStatus;
            wait(rightChild, &rightStatus, -1); // wait for the right child
            if (WEXITSTATUS(rightStatus) == 1){ // command not executed by right child
                return;
            }
            
            //at this point the 2 commands were executed, so gather statistics
            totalCommands += 2;            
        }
    }
}

/**
 * char * line: the input sentence from the user, max of MAX_LINE_LENGTH characters
 * char *** historyList a pointer to historyList, since we will be making changes to it (adding new strings)
 *
 * analyzeLine scans line for words/commands, then executes the command by calling  executeCommand(int, char**).
 * then, saves the words/commands to historyList and expands historyList if necessary
*/
void analyzeLine(char* line, char*** historyList) {
    //Expand historyList if necessary.
    if (historyCapacity >= historySize) {
        expandList(historyList, historySize, historyCapacity, EXPAND_LIST_MULTIPLIER);
    }
    int wordCount = 0;
    int startingIndex = 0;

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
            //allocate memory for word, gather info about it and free word
            char* word = (char*)malloc(sizeof(char) * (i - startingIndex + 1));
            if (word == NULL) {
                fprintf(stderr, "malloc failed for char* word in analyzeLine()\n");
                deallocateMemory(*historyList, historyCapacity);
                exit(1);
            }
            strncpy(word, line + startingIndex, i - startingIndex);
            word[i - startingIndex] = '\0';
            if (word[0] != '\0'){
                wordCount++;
            }
            free(word);
            //skip all spaces
            while (i < strlen(line) - 1 && line[i + 1] == ' ') {
                i++;
            }
            startingIndex = i + 1;
        }
    }
    
    char * toSaveInHistory = (char*) malloc(sizeof(char) * strlen(line));
    strcpy(toSaveInHistory, line);

    if (line[0] == ' '){
        fprintf(stderr, "invalid command\n");
        free(toSaveInHistory);
        return;
    }

    if (line[0] == '!') {
        //execute history command
        //get index
        char historyIndexString[MAX_LINE_LENGTH + 1];
        strcpy(historyIndexString, line);
        // get first word of line, remove '!' and convert the rest of the string to int
        char* split = strtok(historyIndexString, " ");
        int j = 0; //index of historyIndexString
        for (int i = 0; i < strlen(split); i++) {
            if (split[i] != '!') {
                historyIndexString[j] = split[i];
                j++;
            }
        }
        historyIndexString[j] = '\0';
        int index = atoi(historyIndexString);
        index--;
        //check if user entered an invalid number
        if (index >= 0 && index < historyCapacity) {
            executeCommandHelper(wordCount, (*historyList)[index]);
            strcpy(toSaveInHistory,(*historyList)[index]);
        } else {
            fprintf(stderr, "NOT IN HISTORY\n");
            free(toSaveInHistory);
            return;
        }
    } else {
        //execute normal command
        executeCommandHelper(wordCount, line);
    }

    //check if !<command> is a history command, and dont save it into the history file
    char* toSaveInHistoryNoSpaces = removeSpaces(toSaveInHistory);
    if (strcmp(toSaveInHistoryNoSpaces, "history") == 0 && strstr(toSaveInHistory, "history") != NULL) {
        loadHistory(fileName, historyList);
        printHistory(*historyList);
        free(toSaveInHistoryNoSpaces);

        totalNumOfWordsInCommands--; // history is not considered a command
        totalCommands--;
        return;
    }

    //save toSaveInHistory to historyList.
    (*(historyList))[historyCapacity] = (char*)malloc(sizeof(char) * (strlen(toSaveInHistory) + 1));
    if ((*(historyList))[historyCapacity] == NULL) {
        fprintf(stderr, "malloc failed for (*(historyList))[historyCapacity]\n");
        deallocateMemory(*historyList, historyCapacity);
        exit(1);
    }
    strcpy((*historyList)[historyCapacity], toSaveInHistory);
    historyCapacity++;
    free(toSaveInHistory);
    
}

/**
 * @brief gathers all the information necessary to call executeCommand(int, char**)
 *
 * @param argc number of words in argString, will be our argument count
 * @param argString command string, will be converted to a list of arguments
 */
void executeCommandHelper(int argc, char* argString) {
    // allocate memory to argv since executeCommand needs a pointer to argString
    char temp[MAX_LINE_LENGTH + 1];
    strcpy(temp, argString);
    char** argv = (char**)malloc(sizeof(char*) * (argc + 1));
    if (argv == NULL) {
        fprintf(stderr, "malloc failed for char ** argv");
    }

    //convert argString to argv
    char* split = strtok(temp, " ");
    int i = 0;

    //allocate memory for every single command into argv
    while (split != NULL) {
        argv[i] = (char*)malloc(sizeof(char) * (strlen(split) + 1));
        if (argv[i] == NULL) {
            fprintf(stderr, "malloc failed for char ** argv");
        }
        strcpy(argv[i], split);
        split = strtok(NULL, " ");
        i++;
    }
    // last argument is always NULL
    argv[i] = NULL;
    executeCommand(argc, argv);
    //free argv
    deallocateMemory(argv, argc + 1);
}

/**
 * prints historyList in this format:
 * i: <sentence>
 */
void printHistory(char** historyList) {
    for (int i = 0; i < historyCapacity; i++) {
        printf("%d: %s\n", i+1, historyList[i]);
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
        fprintf(stderr, "malloc failed for *lst in expandList\n");
        exit(1);
    }
    //copy elements of old lst to new lst
    for (int i = 0; i < capacity; i++) {
        (*lst)[i] = (char*)malloc(sizeof(char) * (strlen(temp[i]) + 1));
        if ((*lst)[i] == NULL) {
            fprintf(stderr, "malloc failed for *lst[i] in expandList\n");
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
                fprintf(stderr, "malloc failed for (*historyList)[i] in loadHistory()\n");
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
            fprintf(stderr, "malloc failed for char* temp in saveHistory()\n");
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

/**
 * @brief removes all spaces from str, result string is being allocated
 * 
 * @param str
 * @return a pointer to the result string
 */
char* removeSpaces(char* str){
    char* result = (char*)malloc(sizeof(char) * (strlen(str) + 1));
    if(result == NULL){
        printf("malloc failed for char* result in removeSpaces()\n");
        exit(1);
    }
    int count = 0;
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] != ' '){
            result[count] = str[i];
            count++;
        }
    }
    result[count] = '\0';
    return result; 
}
