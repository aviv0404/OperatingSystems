/*
Name: Aviv Zvuluny
ID: 211313333

Polynomial Calculator V1 - Ex 4:
This program gets an input in this format: "<P>, <V>" from the user, where:
- P is a polynomial in this format: a1*x^b1+a2*x^b2+...+c (Assumes there are no spaces anywhere in P).
- V is a real number
The program then calculates the result of substituting x with <V>
using shared memory and splitting the calculations across multiple child processes.

Compilation:
- Use these arguments: -lpthread, -lm, -lrt
- example: "gcc ex4b.c -o ex4b -lpthread -lm -lrt"

input from the user must not exceed "MAX_LINE_LENGTH" characters
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_LINE_LENGTH 510
#define false 0
#define true 1

void analyzePolynomial(char*, double*, double*, double*);
void scrapePolynomial(char*, char**);
void mainLoop();
void freeAllMemory();
void removeSpaces(char*, char*);
void* threadMain(void*);
int separatePolymonAndValue(char*, char**, char**);
int countCharInString(char*, char);
int getInput(char*, char*);
double calculatePolynomial(char**, int, double);

//making these global so i can free them if an error occurred anywhere
char* polynomialString;    // holds the polynomial in string form
char* valueString;      // holds the value to insert into the polynomial in string form

int main() {
    mainLoop();
    exit(EXIT_SUCCESS);
}

/**
 * @brief main loop of the program, asks for input from the user and prints the result
 *
 */
void mainLoop() {
    char line[MAX_LINE_LENGTH];
    char lineNoSpaces[MAX_LINE_LENGTH];

    int correctFormat = getInput(line, lineNoSpaces);
    while (strcmp(line, "done")) {
        //if done entered
        if (lineNoSpaces == "done" && strstr(line, "done")) {
            break;
        }

        if (!correctFormat) {
            fprintf(stderr, "incorrect format.\nThe format should look like this: \"<Polynomial>, <value>\"\n\n");
            correctFormat = getInput(line, lineNoSpaces);
            continue;
        }

        //get value
        double value = atof(valueString + 1);
        if (value == 0 && valueString[1] != '0') {
            fprintf(stderr, "invalid value.\n\n");
            correctFormat = getInput(line, lineNoSpaces);
            continue;
        }

        // split polynomial elements into a list and calculate them using threads
        int numOfElements = countCharInString(polynomialString, '+');
        char* elementsList[numOfElements + 1];
        scrapePolynomial(polynomialString, elementsList);

        double result = calculatePolynomial(elementsList, numOfElements + 1, value);
        printf("result: %f\n\n", result);

        correctFormat = getInput(line, lineNoSpaces);
    }

    free(polynomialString);
    free(valueString);
}

/**
 * @brief main thread starting point, calculates an element in a polynomial and returns the result of that calculation
 *
 * @param args list of 3 arguments in order to calculate the result: multiplier, power and value
 * @return the result of calculating an element in a polynomial
 */
void* threadMain(void* args) {
    double* result = (double*)malloc(sizeof(double));
    if (result == NULL) {
        perror("malloc failed for result");
        freeAllMemory();
        exit(EXIT_FAILURE);
    }

    double multiplier = *(((double*)args));
    double power = *(((double*)args) + 1);
    double value = *(((double*)args) + 2);

    double temp = pow(value, power);
    *result = multiplier * temp;
    pthread_exit((void*)result);
}

/**
 * @brief calculates the polynomial by spreading the calculations across multiple threads.
 * this method opens the threads and gathers the result
 *
 * @param elementsList list of all elements as strings of the polynomial
 * @param elementsListSize size of elementsList
 * @param value value to insert into the polynomial
 * @return the result of calculating the entire polynomial with the given value
 */
double calculatePolynomial(char* elementsList[], int elementsListSize, double value) {
    double result = 0;
    pthread_t threadList[elementsListSize];
    double args[elementsListSize][3];
    for (int i = 0; i < elementsListSize; i++) {
        double multiplier = 1;
        double power = 1;
        double isXPresent = 0;
        analyzePolynomial(elementsList[i], &multiplier, &power, &isXPresent);
        args[i][0] = multiplier;
        args[i][1] = power;
        args[i][2] = (isXPresent == 0) ? value : isXPresent; //(0 here means x is present)

        pthread_create(&threadList[i], NULL, threadMain, (void*)(args[i]));
    }

    for (int i = 0; i < elementsListSize; i++) {
        double* temp;
        pthread_join(threadList[i], (void**)(&temp));
        result += *temp;
        free(temp);
    }

    return result;
}

/**
 * @brief gets an input from the user
 *
 * @param line string to insert the input to
 * @param lineNoSpaces the input without spaces after a value has been assigned to line
 * @return false if no polynomial or value strings could be created from the input, and true otherwise.
 */
int getInput(char* line, char* lineNoSpaces) {
    printf("Enter a polynomial and a value in this format: \"<Polynomial>, <Value>\" or type \"done\" to exit\n");
    fgets(line, MAX_LINE_LENGTH + 1, stdin);
    fflush(stdin);
    line[strcspn(line, "\n")] = '\0';
    removeSpaces(line, lineNoSpaces);


    if (!separatePolymonAndValue(line, &polynomialString, &valueString) ||
        countCharInString(line, ' ') != 1 ||
        valueString[0] != ' ' ||
        strlen(valueString) < 2)
    {
        return false;
    }
    return true;
}

/**
 * @brief allocates memory for the left and right side of the comma in line
 * which would be the polynomial and the value
 *
 * @param line the input line
 * @param polynomialString result string, 1st return value
 * @param valueString result value string, 2nd return value
 * @return false if there is no comma in line, true and polynomialString, valueString otherwise.
 */
int separatePolymonAndValue(char* line, char** polynomialString, char** valueString) {
    //get the location of the comma, 
    char* temp = strchr(line, ',');
    if (temp == NULL) {
        return false;
    }
    int commaIndex = (int)(temp - line);

    //copy left side to polynomialString and right side to valueString
    *polynomialString = (char*)malloc(sizeof(char) * (commaIndex + 1));
    if (*polynomialString == NULL) {
        perror("malloc failed for polynomialString");
        exit(EXIT_FAILURE);
    }
    *valueString = (char*)malloc(sizeof(char) * (strlen(line) - commaIndex + 1));
    if (*valueString == NULL) {
        perror("malloc failed for valueString");
        free(*polynomialString);
        exit(EXIT_FAILURE);
    }
    strcpy(*valueString, temp + 1);
    strncpy(*polynomialString, line, commaIndex);
    return true;
}

/**
 * @brief gets a line that contains a polynomial and converts it to a list of elements inside the polynomial
 *
 * @param str a string that contains a polynomial
 * @param result a list of elements in the polynomial inside str
 */
void scrapePolynomial(char* str, char* result[]) {
    char* token = strtok(str, "+");
    int resultIndex = 0;
    while (token != NULL) {
        result[resultIndex] = token;
        resultIndex++;
        token = strtok(NULL, "+");
    }
}

/**
 * @brief analyzes one element of a polynomial and returns the multiplier and power of x.
 *  Also returns a value called isXPresent to differentiate constant terms
 *
 * @param element - an element of a polynomial, example: 2*x^3
 * @param multiplier the multiplier of x, example: 2*x^3 => multiplier = 2
 * @param power the power of x, example: 2*x^3 => power = 3
 * @param isXPresent if value is 0 then x is present, otherwise x isn't present and the value of the constant term is isXPresent
 */
void analyzePolynomial(char* element, double* multiplier, double* power, double* isXPresent) {
    //check if x is in element
    char* xChar = strchr(element, 'x'); // substring of element from x
    if (xChar == NULL) {
        *multiplier = 1;
        *power = 1;
        *isXPresent = atof(element);
        return;
    }
    else {
        *isXPresent = 0;
    }

    char* multiplierChar = strchr(element, '*'); // substring of element from *
    char* powerChar = strchr(element, '^'); // substring of element from ^

    if (multiplierChar == NULL && powerChar != NULL) {
        *multiplier = 1;

        *power = atof(powerChar + 1);

    }
    else if (multiplierChar != NULL && powerChar == NULL) {
        *power = 1;

        int multiplierIndex = (int)(multiplierChar - element); // index of asterisk
        char multiplierString[multiplierIndex + 1]; // multiplier in string form
        strncpy(multiplierString, element, multiplierIndex);
        *multiplier = atof(multiplierString);

    }
    else if (multiplierChar != NULL && powerChar != NULL) {

        int multiplierIndex = (int)(multiplierChar - element); // index of asterisk
        char multiplierString[multiplierIndex + 1]; // multiplier in string form
        strncpy(multiplierString, element, multiplierIndex);
        *multiplier = atof(multiplierString);

        *power = atof(powerChar + 1);

    }
    else {
        *multiplier = 1;
        *power = 1;
    }



}

/**
 * @brief counts how many times ch appears in str
 *
 * @param str an input string
 * @param ch a char to search in str
 * @return the count
 */
int countCharInString(char* str, char ch) {
    int count = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        count += (str[i] == ch) ? 1 : 0;
    }
    return count;
}

/**
 * @brief frees all global dynamically allocated variables (using malloc)
 *
 */
void freeAllMemory() {
    free(polynomialString);
    free(valueString);
}

/**
 * @brief removes all spaces from str, and puts the result in dest.
 *  assumes dest has enough spaces for the result string
 *
 * @param str input string to remove spaces in.
 * @param dest result string will be put in here
 */
void removeSpaces(char* src, char* dest) {
    int index = 0;
    for (int i = 0; i < strlen(src); i++) {
        if (src[i] != ' ') {
            dest[index] = src[i];
            index++;
        }
    }
}