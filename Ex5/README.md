ID: 211313333
Name: Aviv Zvuluny

Memory Simulation - Ex 5

Description:
    - This program simulates how the operating system would connect the logcial and the physical memory of processes

    - The 2 main functions for the user are load and store, where load returns a 
      byte (represented as a char in this simulation) from the physical memory, and store sets a byte in the physical memory.

    - In order to navigate, and refer to each page individually, these commands are following the 
      page flow chart, before the actual store/load command is executed 
      
    - The method used for finding the right page is division and modulo, this is an 
      inaccurate method to find the correct page, which means the logical address size is different than in reality.
      (in depth explanation in the documentation of getPageFromAddress(int address, int page_size, int num_of_pages))


Program DATABASE:
    - The main ways to store information in this program are:
        1) a char list: char main_memory[MEMORY_SIZE] that contains all the simulated physical memory
        2) a page_descriptor list: page_table[num_of_proc][num_of_pages] contains info on every page of each process

    - 3 files:
        1) swap_file_name: = contains all the memory that will be dumped to this file when the memory is full
        2) exe_file_name1, exe_file_name2: 2 files that contain the text and data memory sections of each process. 
            if num_of_proc = 2 both of them have to exist. if num_of_proc = 1 then only 
            1 must exist. if those 2 conditions aren't met, the program won't start.

Functions:
    - void copyToMainMem(int, int, char *); - copies a page of memory to the physical memory

    - void mainMemToSwap(); - copies a page in the physical memory to the swap file

    - void deleteFromSwap(int, int); - deletes a given page from the swap file

    - int logicalMemToPhysical(int, int); - gets a logical address and returns the frame in memory for that specific address

    - int getPageFromFrame(int, int *); - gets a frame and returns the page that holds this frame inside any of the page tables from all processes

    - int findSwapSpace(); - finds the index of a free space for a page inside the swap file

    - char pageFlowChart(int, int, int, char); - the main way to navigate pages in order to put them in the memory correctly

    - char executeCommand(int, int, int, int, int); - executes either load or store on the physical memory based on the arguments given

    - char load(int, int); - the bread and butter of this program, returns a byte in the physical memory based on the address given

    - void store(int, int, char); - the bread and butter of this program, sets a byte in the physical memory in the address given to the value given

    - void print_memory(); - prints the physical memory in a certain format

    - void print_swap(); - prints all the pages inside the swap file

    - void print_page_table(); - prints the page tables of all the opened processes


Program Files
    - main.cpp - main start point, used for testing the program
    - sim_mem.cpp - contains the implemenetation of all the functions
    - sim_mem.h - contains all the function, class declarations and includes

compile and run (Linux):
    - compile: g++ main.cpp sim_mem.h sim_mem.cpp -o main
    - run: ./main

Input:
    - load: a logical address to write to, and the process id of that logical address (process id starts at 0)
    - store: a logical address to write to, the process id of that logical address and a value to insert into the memory. (process id starts at 0)
    - main.cpp: the program has no input

Output:
    - load: either the char the resides in the given logical address or a '\0' indicating an error.
    - main.cpp: the result of some testing, prints the main memory, the page table of each process and the swap file of an example.

