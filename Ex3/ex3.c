/*
Name: Aviv Zvuluny
ID: 211313333

Ex 3: Shell Simulator+
This program simulates the linux bash command line.
The program supports normal commands, single pipe commands, double pipe commands, ampersand commands and nohup commands.
The program gets commands from the user and executes them until the word "done" is received.
The user can enter the word "history" to see a list of all.
The commands entered by them in current and previous sessions of using the program.
Every command is being saved into a file named "history", and info on all the commands will be printed when the user enters "done".

Added features:
    - support for a nohup command
    - supprot for an ampersand command
    - support for one, or two pipes

key words (features supported):
    done
    history
    !<previousCommandIndex>
    nohup <command>
    <command> &

input from the user must not exceed "MAX_LINE_LENGTH" characters
*/
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_LINE_LENGTH 512         // the max length of the input string from the user (excluding '\0')
#define EXPAND_LIST_MULTIPLIER 1.3  // arbitrary 1.3, nothing special about it, it just felt right (warning: don't go below 1)

#define PIPE_WRITE_END 1            // pipe write end constant
#define PIPE_READ_END 0             // pipe read end constant

#define LINE_INDEX 0                // indexes for lineList, they represent what string is in lineList[i]
#define LEFT_INDEX 1
#define MID_INDEX 2
#define RIGHT_INDEX 3

#define NOHUP_FILENAME "nohup.out"  // file to redirect input to when nohup command is found.

//functions
void loadHistory(char*, char***);
void saveHistory(char*, char**);
void printHistory(char**);
void expandList(char***, int, int, double);
void deallocateMemory(char**, int);
void analyzeLine(char**, char***, int);
void getCommands();
void executeCommandHelper(char**, int, char***);
void executeCommand(char**, int);
void executeSinglePipeCommand(char**, char**, int);
void executeDoublePipeCommand(char**, char**, char**, int);
void getInput(char*, char**);
void generateArgv(char*, int, char***);
void sigChildHandler();
int isEmpty(char*);
int countCharInString(char, char*);
int countWords(char*, char***);
int convertExclamationsToCommands(char*, char*, char***);
char* removeSpaces(char*);

int totalCommands = 0; // the number of words in all commands

int totalPipes = 0; // total number of pipes across the life of the program

int nohupFlag = 0;     // raised when user entered nohup before a command

int ampersandCommand = 0; // raised when the user entered the ampersand character at the end of a commands

int historyCapacity = 0;    // capacity is the number of elements in historyList
int historySize = 10;       // starting with an arbitrary size of 10, will expand if necessary
char* fileName = "history"; // history file name
char path[MAX_LINE_LENGTH]; // full path to current folder

/**
 * start of process, calls getCommands where the main loop starts.
 * at the end prints statistics about the user's commands.
 */
int main() {
    //signal SIGCHLD
    signal(SIGCHLD, sigChildHandler);

    getCommands();
    //print statistics
    printf("Num of commands: %d\n", totalCommands);
    printf("Total number of pipe commands used: %d !\n\n", totalPipes);
    return 0;
}

/**
 * @brief Handles a process that sends SIGCHLD in order to retrieve its memory and resources.
 * Used exclusively for an ampersand command.
 */
void sigChildHandler() {
    waitpid(-1, NULL, WNOHANG | WUNTRACED);
}

/**
 * @brief uses fork + execvp to execute the input command in argv
 *
 * @param ampersandCommand ampersand flag. if set, send process to the background.
 * @param argv argument values
 */
void executeCommand(char* argv[], int ampersandCommand) {

    char path[MAX_LINE_LENGTH];
    pid_t child = fork();
    if (child == 0) {
        //check for "cd" command
        if (strcmp(argv[0], "cd") == 0) {
            printf("command not supported (Yet)\n");
        }
        else {
            if (nohupFlag) {
                //open nohup file with create, read/write, append 
                //(for eof pointer) and give the user permissions to open and edit it
                int nohupFd = open(NOHUP_FILENAME, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR);
                //redirect stdout to nohup.out
                dup2(nohupFd, STDOUT_FILENO);
            }
            execvp(argv[0], argv);
            fprintf(stderr, "execvp failed, command not found\n");
            exit(EXIT_FAILURE);
        }
        exit(1);
    }
    else if (child == -1) {
        perror("\nfork error");
    }
    else {
        if (!ampersandCommand) {
            signal(SIGCLD, SIG_DFL);
            int status;
            waitpid(child, &status, 0);
            if (!WIFEXITED(status) && !WIFSIGNALED(status)) {
                fprintf(stderr, "Child %d ended abruptly with status: %d\n", child, WEXITSTATUS(status));
            }
            signal(SIGCHLD, sigChildHandler);
        }
    }
}

/**
 * main loop.
 * @brief Each time asks for an input from the user, splits the line into left mid and right sections in case a pipe was entered,
 * analyzes the line/command, saves to history file and repeats
 *
 * if the word "done" is entered, exits process and frees all allocated memory
*/
void getCommands() {
    int argc;
    char** historyList = (char**)malloc(sizeof(char*) * historySize);

    char* lineList[4]; // list of lines to send to analyzeLine(...)

    if (historyList == NULL) {
        fprintf(stderr, "malloc failed for historyList in main()\n");
    }
    //load history from file to historyList
    loadHistory(fileName, &historyList);

    //get first input
    printf("%s> ", getcwd(path, sizeof(path)));
    char in[MAX_LINE_LENGTH + 1];
    fgets(in, MAX_LINE_LENGTH + 1, stdin);
    fflush(stdin);// in case of excess input (line is longer than MAX_LINE_LENGTH), clean the buffer
    in[strlen(in) - 1] = '\0';
    char* inNoSpaces = removeSpaces(in);
    //main loop
    while (strcmp(inNoSpaces, "done") != 0 || strstr(in, "done") == NULL) {
        //if input is whitespace or \0 alone, get a new input
        if (isEmpty(in) == 0) {
            getInput(in, &inNoSpaces);
            continue;
        }

        //convert !<command> to a real command
        char line[MAX_LINE_LENGTH * 4]; // multiply by 4 because we can have 4 exclamation commands with 
                                        //length of MAX_LINE_LENGTH each and then the command 
                                        //itself may be MAX_LINE_LENGTH
        int status = convertExclamationsToCommands(in, line, &historyList);
        if (status == 0) {
            //success, print how the new command looks like to the user
            printf("%s\n\n", line);
        }
        else if (status == 1) {
            //not in history failure. get new input
            getInput(in, &inNoSpaces);
            continue;
        }

        //detect nohup
        if (strncmp("nohup", line, strlen("nohup")) == 0) {
            //remove nohup from line, raise flag
            char* temp = strchr(line, ' ') + 1;
            strcpy(line, temp);

            nohupFlag = 1;
            //ignore hup signal
            signal(SIGHUP, SIG_IGN);
        }
        else {
            nohupFlag = 0;
            //resume hup signal
            signal(SIGHUP, SIG_DFL);
        }

        // check for ampersand
        ampersandCommand = 0;
        char* temp = removeSpaces(line);
        if (temp[strlen(temp) - 1] == '&') {
            ampersandCommand = 1;
            //find & and replace by '\0'
            for (int i = strlen(line) - 1; i >= 0; i--) {
                if (line[i] == '&') {
                    line[i] = '\0';
                    break;
                }
            }
        }
        free(temp);

        //count pipes, then analyze the string based on the amount of pipes
        int numOfPipes = countCharInString('|', line);
        if (numOfPipes == 2) {
            // two pipes

            // generate left string
            char* temp = strstr(line, "|");
            int firstPipeIndex = (int)(temp - line);
            char* left = (char*)malloc(sizeof(char) * (firstPipeIndex + 1));
            if (left == NULL) {
                fprintf(stderr, "malloc failed for char * left");
                exit(EXIT_FAILURE);
            }
            left[firstPipeIndex] = '\0';
            strncpy(left, line, firstPipeIndex);

            // generate mid string
            char* temp2 = strstr(temp + 1, "|");
            int secondPipeIndex = (int)(temp2 - line);
            char* mid = (char*)malloc(sizeof(char) * (secondPipeIndex - firstPipeIndex));
            if (mid == NULL) {
                fprintf(stderr, "malloc failed for char * mid");
                free(left);
                exit(EXIT_FAILURE);
            }
            mid[secondPipeIndex - firstPipeIndex - 1] = '\0';
            strncpy(mid, temp + 1, secondPipeIndex - firstPipeIndex - 1);

            //generate right string
            char* right = (char*)malloc(sizeof(char) * (strlen(line) - secondPipeIndex));
            if (right == NULL) {
                fprintf(stderr, "malloc failed for char * mid");
                free(left);
                free(mid);
                exit(EXIT_FAILURE);
            }
            right[strlen(line) - secondPipeIndex - 1] = '\0';
            strncpy(right, temp2 + 1, strlen(line) - secondPipeIndex - 1);

            //analyze string
            lineList[LINE_INDEX] = line;
            lineList[LEFT_INDEX] = left;
            lineList[MID_INDEX] = mid;
            lineList[RIGHT_INDEX] = right;
            analyzeLine(lineList, &historyList, numOfPipes);

            //free memory
            free(left);
            free(mid);
            free(right);
        }
        else if (numOfPipes == 1) {
            //one pipe

            // generate left string
            char* temp = strstr(line, "|");
            int pipeIndex = (int)(temp - line);
            char* left = (char*)malloc(sizeof(char) * (pipeIndex + 1));
            if (left == NULL) {
                fprintf(stderr, "malloc failed for char * left");
                exit(EXIT_FAILURE);
            }
            left[pipeIndex] = '\0';
            strncpy(left, line, pipeIndex);

            // generate right string
            char* right = (char*)malloc(sizeof(char) * (strlen(line) - pipeIndex));
            if (right == NULL) {
                fprintf(stderr, "malloc failed for char * right");
                free(left);
                exit(EXIT_FAILURE);
            }
            right[strlen(line) - pipeIndex] = '\0';
            strncpy(right, temp + 1, strlen(line) - pipeIndex);

            // analyze lines
            lineList[LINE_INDEX] = line;
            lineList[LEFT_INDEX] = left;
            lineList[MID_INDEX] = NULL;
            lineList[RIGHT_INDEX] = right;
            analyzeLine(lineList, &historyList, numOfPipes);

            //free memory
            free(left);
            free(right);
        }
        else if (numOfPipes == 0) {
            //no pipe
            //analyze words (print them and count words) and then save to the history file
            lineList[LINE_INDEX] = line;
            lineList[LEFT_INDEX] = NULL; lineList[MID_INDEX] = NULL; lineList[RIGHT_INDEX] = NULL;
            analyzeLine(lineList, &historyList, numOfPipes);
        }
        else {
            fprintf(stderr, "too many pipes, command not supported.\n");
            getInput(in, &inNoSpaces);
            continue;
        }
        saveHistory(fileName, historyList);
        getInput(in, &inNoSpaces);
    }
    //free memory
    free(inNoSpaces);
    deallocateMemory(historyList, historyCapacity);
}

/**
 * @brief counts occurrences of ch in str.
 * 
 * @param ch a char to count
 * @param str source string
 * @return the number of times the char ch is in the string str.
 */
int countCharInString(char ch, char* str) {
    int result = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == ch) {
            result++;
        }
    }
    return result;
}

/**
 * @brief gets input from the user
 * 
 * @param line string to send the input to
 * @param lineNospaces used in getCommands(), its line but without spaces
 */
void getInput(char* line, char** lineNospaces) {
    printf("%s> ", getcwd(path, sizeof(path)));
    fgets(line, MAX_LINE_LENGTH + 1, stdin);
    fflush(stdin);
    line[strcspn(line, "\n")] = '\0';
    free(*lineNospaces);
    *lineNospaces = removeSpaces(line);
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
        if (str[i] != ' ' && str[i] != '\0' && str[i] != '\n') {
            return 1;;
        }
        i++;
    }
    return 0;
}

/**
 * @brief execute a command with 2 pipes (executes 3 commands in total).
 * 
 * @param argvLeft arguments for the leftmost command
 * @param argvMid arguments for the commad in the middle
 * @param argvRight arguments for the rightmost command
 * @param ampersandCommand whether to send rightmost command to the background
 */
void executeDoublePipeCommand(char* argvLeft[], char* argvMid[], char* argvRight[], int ampersandCommand) {
    //check for "cd" command
    if (strcmp(argvLeft[0], "cd") == 0 || strcmp(argvRight[0], "cd") == 0 || strcmp(argvMid[0], "cd") == 0) {
        printf("command not supported (Yet)\n");
        return;
    }

    //create pipes
    int fds1[2];
    int fds2[2];
    if (pipe(fds1) == -1 || pipe(fds2) == -1) {
        perror("error creating pipe\n");
        exit(EXIT_FAILURE);
    }
    // fork left child (left child will execute argvLeft)
    pid_t leftChild = fork();
    if (leftChild == 0) {
        //left child process

        //redirect stdout into pfds[1] (write) and close read.
        dup2(fds1[PIPE_WRITE_END], STDOUT_FILENO);

        // close unnecessary connections to the pipes
        close(fds1[PIPE_WRITE_END]);
        close(fds1[PIPE_READ_END]);

        execvp(argvLeft[0], argvLeft);
        fprintf(stderr, "execvp failed, command not found\n");
        exit(EXIT_FAILURE);
    }
    else if (leftChild == -1) {
        close(fds2[PIPE_READ_END]);
        close(fds1[PIPE_READ_END]);
        close(fds2[PIPE_WRITE_END]);
        close(fds1[PIPE_WRITE_END]);
        perror("fork error\n");
    }

    pid_t midChild = fork();
    if (midChild == 0) {
        //mid child process

        //redirect read and write to pipe
        dup2(fds1[PIPE_READ_END], STDIN_FILENO);
        dup2(fds2[PIPE_WRITE_END], STDOUT_FILENO);

        // close unnecessary connections to the pipes
        close(fds2[PIPE_READ_END]);
        close(fds1[PIPE_READ_END]);
        close(fds2[PIPE_WRITE_END]);
        close(fds1[PIPE_WRITE_END]);

        //execute mid
        execvp(argvMid[0], argvMid);
        fprintf(stderr, "execvp failed, command not found\n");
        exit(EXIT_FAILURE);
    }
    else if (midChild == -1) {
        close(fds2[PIPE_READ_END]);
        close(fds1[PIPE_READ_END]);
        close(fds2[PIPE_WRITE_END]);
        close(fds1[PIPE_WRITE_END]);
        perror("\nfork error\n");
    }

    // close unnecessary connections to the pipes
    close(fds1[PIPE_READ_END]);
    close(fds1[PIPE_WRITE_END]);

    pid_t rightChild = fork();
    if (rightChild == 0) {
        //right child process

        //redirect stdin into pfds[0] (read)
        dup2(fds2[PIPE_READ_END], STDIN_FILENO);

        // close unnecessary connections to the pipes
        close(fds2[PIPE_READ_END]);
        close(fds2[PIPE_WRITE_END]);

        //execute right
        execvp(argvRight[0], argvRight);
        fprintf(stderr, "execvp failed, command not found\n");
        exit(EXIT_FAILURE);
    }
    else if (rightChild == -1) {
        close(fds2[PIPE_READ_END]);
        close(fds1[PIPE_READ_END]);
        close(fds2[PIPE_WRITE_END]);
        close(fds1[PIPE_WRITE_END]);
        perror("\nfork error\n");
    }

    //parent process, wait for children to execute, close pipes
    close(fds2[PIPE_READ_END]);
    close(fds2[PIPE_WRITE_END]);
    
    if (!ampersandCommand) {
        //stop sigchld handling for background processes
        signal(SIGCLD, SIG_DFL);
        //wait for children
        int leftStatus;
        waitpid(leftChild, &leftStatus, WUNTRACED);
        if (!WIFEXITED(leftStatus) && !WIFSIGNALED(leftStatus) && !WIFSTOPPED(leftStatus)) {
            fprintf(stderr, "Child %d ended abruptly with status: %d\n", leftChild, WEXITSTATUS(leftStatus));
            exit(EXIT_FAILURE);
        }
        int midStatus;
        waitpid(midChild, &midStatus, WUNTRACED);
        if (!WIFEXITED(midStatus) && !WIFSIGNALED(midStatus) && !WIFSTOPPED(midStatus)) {
            fprintf(stderr, "Child %d ended abruptly with status: %d\n", midChild, WEXITSTATUS(midStatus));
            exit(EXIT_FAILURE);
        }
        int rightStatus;
        waitpid(rightChild, &rightStatus, WUNTRACED);
        if (!WIFEXITED(rightStatus) && !WIFSIGNALED(rightStatus) && !WIFSTOPPED(rightStatus)) {
            fprintf(stderr, "Child %d ended abruptly with status: %d\n", rightChild, WEXITSTATUS(rightStatus));
            exit(EXIT_FAILURE);
        }
        //resume sigchld handling for background processes
        signal(SIGCHLD, sigChildHandler);
    }
}

/**
 * @brief execute a command that contains a single pipe (executes 2 commands in total)
 * 
 * @param argvLeft left command arugments
 * @param argvRight right command arguments
 * @param ampersandCommand whether to send right command to the background or not
 */
void executeSinglePipeCommand(char* argvLeft[], char* argvRight[], int ampersandCommand) {
    //check for "cd" command
    if (strcmp(argvLeft[0], "cd") == 0 || strcmp(argvRight[0], "cd") == 0) {
        printf("command not supported (Yet)\n");
        return;
    }

    //create pipe
    int pfds[2];
    if (pipe(pfds) == -1) {
        perror("failed while creating pipe\n");
        exit(EXIT_FAILURE);
    }
    // fork left child (left child will execute argvLeft)
    pid_t leftChild = fork();
    if (leftChild == 0) {
        //left child process
        //redirect stdout into pfds[1] (write) and close unused pipes
        dup2(pfds[PIPE_WRITE_END], STDOUT_FILENO);
        close(pfds[PIPE_READ_END]);
        close(pfds[PIPE_WRITE_END]);

        execvp(argvLeft[0], argvLeft);
        fprintf(stderr, "execvp failed, command not found\n");
        exit(EXIT_FAILURE);
    }
    else if (leftChild == -1) {
        perror("fork error\n");
    }
    else
    {
        //wait for child
        int leftStatus;
        waitpid(leftChild, &leftStatus, 0);
        if (!WIFEXITED(leftStatus) && !WIFSIGNALED(leftStatus)) {
            fprintf(stderr, "Child %d ended abruptly with status: %d\n", leftChild, WEXITSTATUS(leftStatus));
            exit(EXIT_FAILURE);
        }
        // close parent pipes

        pid_t rightChild = fork();
        if (rightChild == 0) {
            //right child process
            //redirect stdin into pfds[0] (read) and close unused pipes
            dup2(pfds[PIPE_READ_END], STDIN_FILENO);
            close(pfds[PIPE_WRITE_END]);
            close(pfds[PIPE_READ_END]);

            //execute right
            execvp(argvRight[0], argvRight);
            fprintf(stderr, "execvp failed, command not found\n");
            exit(EXIT_FAILURE);
        }
        else if (rightChild == -1) {
            perror("\nfork error\n");
        }
        else
        {
            //parent process, wait for left child to execute command, close pipes
            close(pfds[PIPE_READ_END]);
            close(pfds[PIPE_WRITE_END]);

            if (!ampersandCommand) {
                //stop sigchld handling
                signal(SIGCLD, SIG_DFL);
                //wait for child
                int rightStatus;
                waitpid(rightChild, &rightStatus, 0);
                if (!WIFEXITED(rightStatus) && !WIFSIGNALED(rightStatus)) {
                    fprintf(stderr, "Child %d ended abruptly with status: %d\n", rightChild, WEXITSTATUS(rightStatus));
                    exit(EXIT_FAILURE);
                }
                //return sigchld handling
                signal(SIGCHLD, sigChildHandler);
            }
        }
    }
}

/**
 * @brief counts how many words (separated by spaces) are in line.
 * 
 * @param line string to search words in
 * @param historyList for deallocation purposes
 * @return number of words in line.
 */
int countWords(char* line, char*** historyList) {
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
            if (word[0] != '\0') {
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
    return wordCount;
}

//converts an exclamation command (!<command number>) to a regular command and stores the result in dest
//assumes dest is the same length as src
//returns -1 if no !<command>s were found in src, 0 if everything worked correctly and 1 if not in history

/**
 * @brief converts all exclamation commands (!<index of command>) 
 * to a regular command from history list. and stores the result in dest. 
 * Also assumes dest is 4 times longer than src because a 
 * command can be MAX_LINE_LENGTH long and contain 3 commands of length MAX_LINE_LENGTH.
 * 
 * @param src source string
 * @param dest result string
 * @param historyList 
 * @return -1 if no ! commands were found, 1 if index is not in history and 0 if everything was successful.
 */
int convertExclamationsToCommands(char* src, char* dest, char*** historyList) {
    strcpy(dest, src);
    char* split = strchr(src, '!');
    if (split == NULL) {
        return -1;
    }
    int destIndex = 0;
    int exclamationIndex = 0;
    char* temp = NULL;
    while (split != NULL) {
        exclamationIndex = (int)(split - src);
        //copy everything up to exclamation
        int tempIndex = (temp == NULL) ? 0 : (int)(temp - src);
        for (int i = tempIndex; i < exclamationIndex; i++) {
            dest[destIndex] = src[i];
            destIndex++;
        }

        //convert exclamation to history command
        temp = strchr(split, ' ');                  //index of next space after exclamation
        //if no space found, that means we're at the end of src
        if (temp == NULL) {
            temp = src + strlen(src);
        }
        int numberLength = (int)(temp - (split + 1));  //length of number
        char num[numberLength + 1];                    // string that holds the number
        strncpy(num, split + 1, numberLength);
        int historyIndex = atoi(num);
        historyIndex--; // because we show history indexes from 1 and not 0 to the user
        // if index is valid
        if (historyIndex >= 0 && historyIndex < historyCapacity) {
            //copy to dest
            for (int i = 0; i < strlen((*historyList)[historyIndex]); i++) {
                dest[destIndex] = (*historyList)[historyIndex][i];
                destIndex++;
            }
        }
        else {
            fprintf(stderr, "NOT IN HISTORY\n");
            return 1;
        }
        split = strchr(split + 1, '!');
    }

    //copy the rest of the string to dest
    int tempIndex = (int)(temp - src);
    for (int i = tempIndex; i < strlen(src); i++) {
        dest[destIndex] = src[i];
        destIndex++;
    }

    dest[destIndex] = '\0';
    return 0;
}

/**
 * @param lineList: refers to the commands sent.
 * [0] = full line, [1] = left command [2] = middle command [3] = right command
 *
 * @param historyList a pointer to historyList, since we will be making changes to it (adding new strings)
 * 
 * @param numOfPipes number of pipes to send to executeCommandHelper(...)
 *
 * @brief analyzeLine executes the command by calling executeCommandHelper(...)
 * and then saves the commands to historyList (also expands historyList if necessary).
 *
 * if the word "history" is entered, loads all the history from fileName to historyList and print it to the user.
*/
void analyzeLine(char* lineList[], char*** historyList, int numOfPipes) {
    //Expand historyList if necessary.
    if (historyCapacity >= historySize) {
        expandList(historyList, historySize, historyCapacity, EXPAND_LIST_MULTIPLIER);
    }
    char* toSaveInHistory = (char*)malloc(sizeof(char) * strlen(lineList[LINE_INDEX]));
    strcpy(toSaveInHistory, lineList[LINE_INDEX]);

    //check if <command> is a history command, dont execute it with exec functions.
    char* toSaveInHistoryNoSpaces = removeSpaces(toSaveInHistory);
    if (strcmp(toSaveInHistoryNoSpaces, "history") == 0 && strstr(toSaveInHistory, "history") != NULL) {
        loadHistory(fileName, historyList);
        printHistory(*historyList);
        free(toSaveInHistoryNoSpaces);
    }
    else {
        // execute command
        executeCommandHelper(lineList, numOfPipes, historyList);
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

// converts a line to a list of string arguments, stores the result in the 3rd argument
// assumes result is 2D dynamically allocated array

/**
 * @brief converts a line to a list of string arguments and stores the result in the 3rd argument.
 * assumes result is a 2D dynamically allocated array.
 * 
 * @param line a line of commands to generate arguments from
 * @param argc number of commands in line (assumed to be counted already)
 * @param result list of strings to store the result. last index will always contain NULL.
 */
void generateArgv(char* line, int argc, char*** result) {
    char temp[MAX_LINE_LENGTH + 1];
    strcpy(temp, line);
    //convert argString to argv
    char* split = strtok(temp, " ");
    int i = 0;

    //allocate memory for every single command into argv
    while (split != NULL) {
        (*result)[i] = (char*)malloc(sizeof(char) * (strlen(split) + 1));
        if ((*result)[i] == NULL) {
            fprintf(stderr, "malloc failed for char ** argv");
            deallocateMemory(*result, argc + 1);
            exit(EXIT_FAILURE);
        }
        strcpy((*result)[i], split);
        split = strtok(NULL, " ");
        i++;
    }
    // last argument is always NULL
    (*result)[i] = NULL;
}

/**
 * @brief gathers all the information necessary to call executeCommand(...)
 *
 * @param lineList list of commands to execute, lengthier explanation in analyzeLine(...)
 * @param numOfPipes each number of pipes redirects to a different function call
 * @param historyList a pointer to history list
 */
void executeCommandHelper(char* lineList[], int numOfPipes, char*** historyList) {

    if (numOfPipes == 0) {
        // no pipe
        int wordCount = countWords(lineList[LINE_INDEX], historyList);
        // allocate memory to argv since executeCommand needs a pointer to argString
        char** argv = (char**)malloc(sizeof(char*) * (wordCount + 1));
        if (argv == NULL) {
            fprintf(stderr, "malloc failed for char ** argv");
            deallocateMemory(*historyList,historyCapacity);
            exit(EXIT_FAILURE);
        }
        generateArgv(lineList[LINE_INDEX], wordCount, &argv);
        executeCommand(argv, ampersandCommand);
        deallocateMemory(argv, wordCount + 1);
    }
    else if (numOfPipes == 1) {
        // one pipe
        int leftWordCount = countWords(lineList[LEFT_INDEX], historyList);
        int rightWordCount = countWords(lineList[RIGHT_INDEX], historyList);

        char** leftArgv = (char**)malloc(sizeof(char*) * (leftWordCount + 1));
        if (leftArgv == NULL) {
            fprintf(stderr, "malloc failed for char ** leftArgv");
            exit(EXIT_FAILURE);
        }
        char** rightArgv = (char**)malloc(sizeof(char*) * (rightWordCount + 1));
        if (rightArgv == NULL) {
            fprintf(stderr, "malloc failed for char ** rightArgv");
            free(leftArgv);
            exit(EXIT_FAILURE);
        }
        generateArgv(lineList[LEFT_INDEX], leftWordCount, &leftArgv);
        generateArgv(lineList[RIGHT_INDEX], rightWordCount, &rightArgv);

        executeSinglePipeCommand(leftArgv, rightArgv, ampersandCommand);

        deallocateMemory(leftArgv, leftWordCount + 1);
        deallocateMemory(rightArgv, rightWordCount + 1);

        totalPipes++;
    }
    else {
        //two pipes
        int leftWordCount = countWords(lineList[LEFT_INDEX], historyList);
        int midWordCount = countWords(lineList[MID_INDEX], historyList);
        int rightWordCount = countWords(lineList[RIGHT_INDEX], historyList);

        char** leftArgv = (char**)malloc(sizeof(char*) * (leftWordCount + 1));
        if (leftArgv == NULL) {
            fprintf(stderr, "malloc failed for char ** leftArgv");
            exit(EXIT_FAILURE);
        }
        char** midArgv = (char**)malloc(sizeof(char*) * (midWordCount + 1));
        if (midArgv == NULL) {
            fprintf(stderr, "malloc failed for char ** midArgv");
            free(leftArgv);
            exit(EXIT_FAILURE);
        }
        char** rightArgv = (char**)malloc(sizeof(char*) * (rightWordCount + 1));
        if (rightArgv == NULL) {
            fprintf(stderr, "malloc failed for char ** rightArgv");
            free(leftArgv);
            free(midArgv);
            exit(EXIT_FAILURE);
        }

        generateArgv(lineList[LEFT_INDEX], leftWordCount, &leftArgv);
        generateArgv(lineList[MID_INDEX], midWordCount, &midArgv);
        generateArgv(lineList[RIGHT_INDEX], rightWordCount, &rightArgv);

        executeDoublePipeCommand(leftArgv, midArgv, rightArgv, ampersandCommand);

        deallocateMemory(leftArgv, leftWordCount + 1);
        deallocateMemory(midArgv, midWordCount + 1);
        deallocateMemory(rightArgv, rightWordCount + 1);

        totalPipes++;
    }
    totalCommands++;
}

/**
 * @brief prints historyList in this format:
 * i: <sentence>
 * 
 * i counts from 1
 */
void printHistory(char** historyList) {
    for (int i = 0; i < historyCapacity; i++) {
        printf("%d: %s\n", i + 1, historyList[i]);
    }
    printf("\n");
}

/**
 * @brief frees all the memory pointed by lst, and the memory allocated for lst
 * int size - size of the list pointed at by lst
 */
void deallocateMemory(char** lst, int size) {
    for (int i = 0; i < size; i++) {
        free(lst[i]);
    }
    free(lst);
}

/**
 * @param lst a pointer to the list to expand, this list will be changed to be the new and bigger list.
 * @param currentSize - size before expanding.
 * @param capacity - how many slots are filled (have memory allocated to them).
 * @param sizeMultiplier - how much to multiply lst's size by.
 *
 * @brief This method allocates more memory for lst to use, used mostly when capacity == currentSize
 *
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
 * @param fileName file to gather information from
 * @param historyList pointer to historyList, since we will be changing it in case capacity == size
 *
 * @brief loads the lines written in fileName to historyList, excluding the new line character
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
 * @param fileName - file to save history to
 * @param historyList - list of strings to save to the file. const since we won't be changing it.
 *
 * @brief saves all the strings stored in historyList to the file fileName
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
 * @brief removes all spaces from str, result string is being allocated.
 *
 * @param str input string to remove spaces in.
 * @return a pointer to the result string.
 */
char* removeSpaces(char* str) {
    char* result = (char*)malloc(sizeof(char) * (strlen(str) + 1));
    if (result == NULL) {
        printf("malloc failed for char* result in removeSpaces()\n");
        exit(1);
    }
    int count = 0;
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] != ' ') {
            result[count] = str[i];
            count++;
        }
    }
    result[count] = '\0';
    return result;
}
