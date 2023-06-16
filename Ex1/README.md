String Parser - Ex1
ID: 211313333
Name: Aviv Zvuluny

Description:
    - The program gets input from the user until the word "exit" is received.

    - The user can enter the word "history" to see a list of all 
      the words/sentences entered by them in current and previous sessions of using the program

    - every other sentence/word is being saved into a file named "history", 
      and info on all the words in the sentence is being printed on the screen to the user

Program DATABASE:
    the main way to store data in this program is by using a char ** list that contains all the strings the user has entered.
    this data is stored in the variable historyList

Functions:
    - void loadHistory(char*, char***);             - loads data from history file to historyList
    - void saveHistory(char*, char** const);        - saves data from historyList to history file
    - void printHistory(char**);                    - prints all the data in historyList
    - void expandList(char***, int, int, double);   - expands historyList when necessary (allocates more memory to it)
    - void deallocateMemory(char**, int);           - deallocates all the memory stored in historyList
    - void analyzeLine(char*, char***);             - analyzes the line entered by the user, and prints info about it.


Program Files
    - parser.c - contains all the functions + implementation
    - history - stores sentences

compile and run (Linux):
    - compile: gcc parser.c -o parser
    - run: ./parser

Input:
    - a string with max of 510 characters (can be changed by changing MAX_LINE_LENGTH)

Output:
    - information about the input

