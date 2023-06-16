Shell Simulator+ - Ex3
ID: 211313333
Name: Aviv Zvuluny

# Description:
    - This program simulates the linux bash command line.
    
    - The program supports normal commands, single pipe commands, double pipe commands, ampersand commands and nohup commands.

    - The program gets commands from the user and executes them until the word "done" is received.

    - The user can enter the word "history" to see a list of all 
      the commands entered by them in current and previous sessions of using the program

    - every command is being saved into a file named "history", 
      and info on all the commands will be printed when the user enters "done".

# Program DATABASE:
    - The main way to store data in this program is by using a char ** list that contains all the strings the user has entered.
      this data is stored in the variable historyList

    - If a command was executed with the "nohup" prefix, the output will be sent to a file called "nohup.out"

    - arguments to execute each commands are being saved in a list called argv, which gets deallocated after each execution

# Functions:
    - void loadHistory(char*, char***);             - loads data from history file to historyList
    - void saveHistory(char*, char** const);        - saves data from historyList to history file
    - void printHistory(char**);                    - prints all the data in historyList
    - void expandList(char***, int, int, double);   - expands historyList when necessary (allocates more memory to it)
    - void deallocateMemory(char**, int);           - deallocates all the memory stored in historyList
    - void analyzeLine(char*, char***);             - analyzes the line entered by the user, and prints info about it.
    
    - void getCommands();                           - main loop, gets a command from the user, 
                                                      executes the command, saves to history and repeat.
    - int isEmpty(char*);                           - helper functions which checks if a string contains only whitespaces and '\0'
    - void getInput(char*, char**);                 - gets input from the user, and stores in the first parameter.
    - void generateArgv(char*, int, char***);       - generates command arguments from a string, created for execvp.
    - void executeCommandHelper(int, char *);       - helps gather all the information needed to call executeCommand from analyzeLine
    - void executeCommand(int, char**);             - executes a shell command by using fork and execvp
    - void sigChildHandler();                       - handles SIGCHLD for when a background process terminates
    - int countCharInString(char, char*);           - counts the number of appearances of a char in a string
    - int countWords(char*, char***);               - counts the amount of words (separated by a space) in a string
    - char* removeSpaces(char*);                    - remove all spaces from a string (only spaces, not white characters)

    - void executeSinglePipeCommand(char**, char**, int);           - executes a command that contains just one pipe, by using one pipe
    - void executeDoublePipeCommand(char**, char**, char**, int);   - executes a command that contains exactly two pipes, by using two pipes. 
    - int convertExclamationsToCommands(char*, char*, char***);     - converts all commands in the format !<command index> to their real command.


# Program Files
    - ex3.c - contains the entire functionality of ex2b.c, and adds support for pipes, nohup and ampersand commands
    - history - stores entered commands

# compile and run (Linux):
    - compile: gcc ex3.c -o ex3
    - run: ./ex3

# Input:
    - a string with max of 512 characters (can be changed by changing MAX_LINE_LENGTH)

# Output:
    - <command entered by the user> for each command
    - <num of commands, num of pipe commands> if the word "done" is entered

