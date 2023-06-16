ID: 211313333
Name: Aviv Zvuluny

Polynomial Calculator - Ex 4

Description:
    - This program gets an input in this format: "<P>, <V>" from the user, where:
        - P is a polynomial in this format: a1*x^b1+a2*x^b2+...+c (Assumes there are no spaces anywhere in P).
        - V is a real number
      The program then calculates the result of substituting x with <V>
      using shared memory and splitting the calculations across multiple child processes.

    - The user can choose to enter "done" to end the program


Program DATABASE:
    - In ex4a.c there is no data stored anywhere but the program's memory
    - In ex4b.c a set of arguments and an int is stored in the shared memory space

Functions:
    - void analyzePolynomial(char*, double*, double*, double*) - analyzes one element of a polynomial and returns the multiplier and power of x.

    - void scrapePolynomial(char*, char**) - converts a string of a polynomial to a list of elements inside the polynomial

    - void mainLoop() - main loop of the program, asks for input from the user and prints the result

    - void freeAllMemory() - frees all global dynamically allocated variables (using malloc)

    - void* threadMain(void*) - calculates an element in a polynomial and returns the result of that calculation

    - int separatePolymonAndValue(char*, char**, char**) - separates the value from the polynomial in line

    - int countCharInString(char*, char) - counts how many times ch appears in str

    - int getInput(char*, char*) - gets an input from the user

    - double calculatePolynomial(char**, int, double) - calculates the polynomial by spreading the calculations across multiple threads or processes


Program Files
    - ex4a.c - polynomial calculator using threads.
    - ex4b.c - polynomial calculator using processes and shared memory.

compile and run (Linux):
    - compile: gcc ex4b.c -o ex4b -lpthread -lm -lrt
    - run: ./ex4b
    and:
    - compile: gcc ex4a.c -o ex4a -lpthread -lm -lrt
    - run: ./ex4a

Input:
    - a string with max of 510 characters (can be changed by changing MAX_LINE_LENGTH)
    - a string in this format: "<P>, <V>" from the user, where:
        - P is a polynomial in this format: a1*x^b1+a2*x^b2+...+c (Assumes there are no spaces anywhere in P).
        - V is a real number

Output:
    - the result of substituting the value in the given polynomial

