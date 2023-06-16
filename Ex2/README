Shell Simulator - Ex2
ID: 211313333
Name: Aviv Zvuluny

Description:
    - The program gets commands from the user and executes them until the word "done" is received.

    - The user can enter the word "history" to see a list of all 
      the commands entered by them in current and previous sessions of using the program

    - every command is being saved into a file named "history", 
      and info on all the commands will be printed when the user enters "done".

Program DATABASE:
    - The main way to store data in this program is by using a char ** list that contains all the strings the user has entered.
      this data is stored in the variable historyList

    - arguments to execute each commands are being saved in a list called argv, which gets deallocated after each execution

Functions:
    - void loadHistory(char*, char***);             - loads data from history file to historyList
    - void saveHistory(char*, char** const);        - saves data from historyList to history file
    - void printHistory(char**);                    - prints all the data in historyList
    - void expandList(char***, int, int, double);   - expands historyList when necessary (allocates more memory to it)
    - void deallocateMemory(char**, int);           - deallocates all the memory stored in historyList
    - void analyzeLine(char*, char***);             - analyzes the line entered by the user, and prints info about it.
    
    - void getCommands();                           - main loop, gets a command from the user, 
                                                      executes the command, saves to history and repeat.

    - void executeCommandHelper(int, char *);       - helps gather all the information needed to call executeCommand from analyzeLine
    - void executeCommand(int, char**);             - executes a shell command by using fork and execvp
    - int isEmpty(char*);                           - helper functions which checks if a string contains only whitespaces and '\0'


Program Files
    - ex1a.c - contains all the functions + implementation of the first part, does not know how to execute history commands
    - ex2a.c - contains all the functions + implementation of the second part, knows how to execute history commands (example: !5)
    - history - stores sentences

compile and run (Linux):
    - compile: gcc parser.c -o parser
    - run: ./parser

Input:
    - a string with max of 510 characters (can be changed by changing MAX_LINE_LENGTH)

Output:
    - <command entered by the user> for each command
    - <num of commands, num of words in command> of all commands if the word "done" is entered

