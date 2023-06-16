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
    g++ main.cpp sim_mem.cpp sim_mem.h -o main
*/

#include "sim_mem.h" //TODO: CHANGE TO .H LATER
#include <iostream>
#include <cmath>

char main_memory[MEMORY_SIZE];
int main()
{

    sim_mem sm((char *)"exec_file1", (char *)"exec_file2", (char *)"swap_file", 25, 50, 25, 25, 25, 5, 2);
    // tests
    sm.load(1, 76);
    sm.load(0, 76);
    sm.store(0, 34, 'M');
    sm.load(0, 61);
    sm.store(0, 53, 'E');
    sm.load(0, 70);
    sm.store(0, 74, 'Q');
    sm.store(1, 98, 'N');
    sm.load(1, 20);
    sm.store(0, 87, 'K');
    sm.store(0, 3, 'P');
    sm.load(1, 114);
    sm.store(1, 46, 'I');
    sm.load(1, 65);
    sm.store(0, 30, 'D');
    sm.load(0, 88);
    sm.store(1, 98, 'W');

    
    sm.print_memory();
    sm.print_page_table();
    sm.print_swap();
}
