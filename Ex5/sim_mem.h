/*
Name: Aviv Zvuluny
ID: 211313333

Memory Simulation - Ex 5:
    This program simulates how the operating system would connect the logcial and the physical memory of processes

    The 2 main functions for the user are load and store, where load returns a 
    byte (represented as a char in this simulation) from the physical memory, and store sets a byte in the physical memory.

    in order to navigate, and refer to each page individually, these commands are following the 
    page flow chart, before the actual store/load command is executed 

Assumptions:
    - the index of processes starts at 0

    - no more than 2 processes can be opened at the same time

    - the method used for finding the right page is division and modulo, this is an 
      inaccurate method to find the correct page, which means the logical address size is different than in reality.
      (in depth explanation in the documentations of this function: getPageFromAddress(int address, int page_size, int num_of_pages))

    - the sum of all text, data, bss, heap, stack pages will be equals to the number of pages 

How to compile: 
    g++ main.cpp sim_mem.h sim_mem.cpp -o main
*/

#ifndef SIM_MEMH
#define SIM_MEMH

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

// for files
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string.h>
#include <iostream>
#include <cmath>
#define MEMORY_SIZE 200
extern char main_memory[MEMORY_SIZE];

typedef struct page_descriptor
{
    int V;          // valid
    int D;          // dirty
    int P;          // permission
    int frame;      // the number of a frame if in case it is page-mapped
    int swap_index; // where the page is located in the swap file.
} page_descriptor;

class sim_mem
{
private:
    int swapfile_fd;   // swap file fd
    int program_fd[2]; // executable file fd
    int text_size;
    int data_size;
    int bss_size;
    int heap_stack_size;
    int num_of_pages;
    int page_size;
    int num_of_proc;
    page_descriptor **page_table; // pointer to page table


    //additional helping variables

    int remove_ind = 0; // index inside main memory of the next page to remove when main memory is full
    int insert_ind = 0; // index inside main memory of where the next page can be insert into.
    int num_of_text_pages = 0;
    int num_of_data_pages = 0;
    int num_of_bss_pages = 0;
    int num_of_heapstack_pages = 0;

    int num_of_valid_pages = 0; // holds the number of pages in the memory at any given time

    //helper functions

    void copyToMainMem(int page, int procId, char *);
    void mainMemToSwap();
    void deleteFromSwap(int page, int procId);
    int logicalMemToPhysical(int processId, int logicalAddress);
    int getPageFromFrame(int frame, int * procId); // procId is the result process id in which the frame was found.
    int findSwapSpace();
    char pageFlowChart(int process_id, int address, int commandType, char value); // commandType: load = 0, store = 1
    char executeCommand(int process_id, int address, int page, int commandType, int value);


public:
    sim_mem(char exe_file_name1[], char exe_file_name2[],
            char swap_file_name[], int text_size, int data_size, int bss_size,
            int heap_stack_size, int num_of_pages, int page_size, int num_of_process);
    ~sim_mem();
    char load(int process_id, int address);
    void store(int process_id, int address, char value);
    void print_memory();
    void print_swap();
    void print_page_table();
};
#endif
