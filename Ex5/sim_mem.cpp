#ifndef SIM_MEMCPP
#define SIM_MEMCPP

#include "sim_mem.h"

#define LOAD_COMM_TYPE 0
#define STORE_COMM_TYPE 1

int getPageFromAddress(int, int, int);

/**
 * @brief prints every byte in the main memory
 *
 */
void sim_mem::print_memory()
{
    int i;
    printf("\n Physical memory\n");
    for (i = 0; i < MEMORY_SIZE; i++)
    {
        printf("[%c]\n", main_memory[i]);
    }
}

/**
 * @brief prints every page, and every byte inside every page in the swap file
 *
 */
void sim_mem::print_swap()
{
    char *str = (char *)malloc(this->page_size * sizeof(char));
    int i;
    printf("\nSwap memory\n");
    lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while (read(swapfile_fd, str, this->page_size) == this->page_size)
    {
        for (i = 0; i < page_size; i++)
        {
            printf("%d - [%c]\t", i, str[i]);
        }
        printf("\n");
    }
    free(str);
}

/**
 * @brief prints all the properties of each page in the page table
 *
 */
void sim_mem::print_page_table()
{
    int i;
    for (int j = 0; j < num_of_proc; j++)
    {
        printf("\npage table of process: %d \n", j);
        printf("Valid\tDirty\tPermission\tFrame\tSwap index\n");
        for (i = 0; i < num_of_pages; i++)
        {
            printf("[%d]\t[%d]\t[%d]\t\t[%d]\t[%d]\n",
                   page_table[j][i].V,
                   page_table[j][i].D,
                   page_table[j][i].P,
                   page_table[j][i].frame,
                   page_table[j][i].swap_index);
        }
    }
}

/**
 * @brief Destroy the sim mem::sim mem object, frees page_table memory and closes all file fds
 *
 */
sim_mem::~sim_mem()
{
    // free pages table
    free(page_table[0]);
    if (num_of_proc == 2)
    {
        free(page_table[1]);
    }
    free(page_table);

    // close all fds
    close(swapfile_fd);
    if (num_of_proc == 1)
    {
        close(program_fd[0]);
    }
    else if (num_of_proc == 2)
    {
        close(program_fd[0]);
        close(program_fd[1]);
    }
}

/**
 * @brief opens the swap and executable files and initializes
 * all variables in the page_table of the number of processes
 *
 * assumes number of processes is either 1 or 2.
 * assumes the number of text, data, bss and heap/stack pages accumulate to num_of_pages
 *
 * initializes the data section to appear right after the text section in the executable files.
 * this way, the page number will indicate the index of the text/data page inside the exe files
 *
 * @param exe_file_name1    - file name of first executable
 * @param exe_file_name2    - file name of second executable
 * @param swap_file_name    - file name of the swap file
 * @param text_size         - the amount of text bytes in the executable files
 * @param data_size         - the amount of data bytes in the executable files
 * @param bss_size          - the amount of bytes the bss memory section contains
 * @param heap_stack_size   - the amount of bytes the heap/stack memory section contains
 *
 * all sizes divided by the page_size will give us the number of pages of each memory section
 *
 * @param num_of_pages      - total number of pages inside page_table (amounts to (text_size + data_size + bss_size + hea_stack_size) * page_size)
 * @param page_size         - number of bytes each page contains
 * @param num_of_process    - number of processes to open (either 1 or 2)
 */
sim_mem::sim_mem(char exe_file_name1[], char exe_file_name2[],
                 char swap_file_name[], int text_size, int data_size, int bss_size,
                 int heap_stack_size, int num_of_pages, int page_size, int num_of_process)
{
    this->num_of_proc = num_of_process;
    this->text_size = text_size;
    this->data_size = data_size;
    this->bss_size = bss_size;
    this->page_size = page_size;
    this->heap_stack_size = heap_stack_size;
    this->num_of_pages = num_of_pages;

    this->num_of_text_pages = text_size / page_size;
    this->num_of_data_pages = data_size / page_size;
    this->num_of_bss_pages = bss_size / page_size;
    this->num_of_heapstack_pages = heap_stack_size / page_size;

    // check num of processes and set program fds
    if (num_of_proc == 1)
    {
        if (access(exe_file_name1, F_OK) == 0 && access(exe_file_name2, F_OK) == -1)
        {
            if (exe_file_name1 == NULL)
            {
                fprintf(stderr, "can't open file, file name of first executable is null\n");
                exit(EXIT_FAILURE);
            }
            else
            {
                // file and whether or not the file exists and if we can read / write to it
                program_fd[0] = open(exe_file_name1, O_RDONLY, 0666);

                page_table = (page_descriptor **)malloc(sizeof(page_descriptor *) * num_of_proc);
                if (page_table == NULL)
                {
                    perror("error in malloc for page_table");
                    exit(EXIT_FAILURE);
                }
                page_table[0] = (page_descriptor *)malloc(sizeof(page_descriptor) * num_of_pages);
                if (page_table[0] == NULL)
                {
                    free(page_table);
                    perror("malloc failed for page_table[0]");
                    exit(EXIT_FAILURE);
                }
            }
        }
        else if (access(exe_file_name2, F_OK) == 0 && access(exe_file_name1, F_OK) == -1)
        {
            if (exe_file_name2 == NULL)
            {
                fprintf(stderr, "can't open file, file name of first executable is null\n");
                exit(EXIT_FAILURE);
            }
            else
            {
                // file and whether or not the file exists and if we can read / write to it
                program_fd[0] = open(exe_file_name2, O_RDONLY, 0666);

                page_table = (page_descriptor **)malloc(sizeof(page_descriptor *) * num_of_proc);
                if (page_table == NULL)
                {
                    perror("error in malloc for page_table");
                    exit(EXIT_FAILURE);
                }
                page_table[0] = (page_descriptor *)malloc(sizeof(page_descriptor) * num_of_pages);
                if (page_table[0] == NULL)
                {
                    free(page_table);
                    perror("malloc failed for page_table[0]");
                    exit(EXIT_FAILURE);
                }
            }
        }
        else
        {
            perror("either exe file doesn't exist, or more exe files exist than requested to open");
            exit(EXIT_FAILURE);
        }
    }
    else if (num_of_proc == 2)
    {
        if (exe_file_name1 == NULL || exe_file_name2 == NULL)
        {
            fprintf(stderr, "can't open file, one or more of the filenames is NULL\n");
            exit(EXIT_FAILURE);
        }
        if (access(exe_file_name1, F_OK) == 0 && access(exe_file_name2, F_OK) == 0)
        {
            program_fd[0] = open(exe_file_name1, O_RDONLY, 0666);
            program_fd[1] = open(exe_file_name2, O_RDONLY, 0666);

            page_table = (page_descriptor **)malloc(sizeof(page_descriptor *) * num_of_proc);
            if (page_table == NULL)
            {
                perror("error in malloc for page_table");
                exit(EXIT_FAILURE);
            }
            page_table[0] = (page_descriptor *)malloc(sizeof(page_descriptor) * num_of_pages);
            if (page_table[0] == NULL)
            {
                free(page_table);
                perror("malloc failed for page_table[0]");
                exit(EXIT_FAILURE);
            }
            page_table[1] = (page_descriptor *)malloc(sizeof(page_descriptor) * num_of_pages);
            if (page_table[0] == NULL)
            {
                free(page_table[0]);
                free(page_table);
                perror("malloc failed for page_table[0]");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            perror("one (or more) of the executable files given does not exist.");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        fprintf(stderr, "error: you must open at least one process and at most 2 processes\n");
        exit(EXIT_FAILURE);
    }

    // open swap file and fill it with pages of 0s
    swapfile_fd = open(swap_file_name, O_RDWR | O_CREAT | O_TRUNC, 0666);
    for (int i = 0; i < num_of_proc * ((page_size * num_of_pages) - text_size); i++)
    {
        write(swapfile_fd, (char *)"0", sizeof(char));
    }

    // init main memory
    for (int i = 0; i < MEMORY_SIZE; i++)
    {
        main_memory[i] = '0';
    }

    // init page_table
    for (int pageNum = 0; pageNum < num_of_pages; pageNum++)
    {
        for (int procNum = 0; procNum < num_of_proc; procNum++)
        {
            page_table[procNum][pageNum].V = 0;
            page_table[procNum][pageNum].D = 0;
            page_table[procNum][pageNum].P = (pageNum < num_of_text_pages) ? 0 : 1;
            page_table[procNum][pageNum].frame = -1;
            page_table[procNum][pageNum].swap_index = -1;
        }
    }

    insert_ind = 0;
    remove_ind = 0;
}

/**
 * @brief loads the correct page that correspondes to the logical memory to the physical memory
 * then sets the value inside the physical memory, by using the page flow chart function.
 *
 * @param process_id - the index of the process the refer to (logical address of that process)
 * @param address    - the logical address to store the value in
 * @param value      - the byte to store in the physical memory
 */
void sim_mem::store(int process_id, int address, char value)
{
    if ((num_of_proc == 1 && process_id != 0) || (num_of_proc == 2 && process_id != 0 && process_id != 1))
    {
        fprintf(stderr, "error: process index out of bounds. process indexes start at 0\n");
        return;
    }
    pageFlowChart(process_id, address, STORE_COMM_TYPE, value);
}

/**
 * @brief loads the correct page that correspondes to the logical memory to the physical memory
 * by using the page flowchart function
 *
 * @param process_id - the index of the process the refer to (logical address of that process)
 * @param address    - the logical address to store the value in
 * @param value      - the byte to store in the physical memory
 */
char sim_mem::load(int process_id, int address)
{
    if ((num_of_proc == 1 && process_id != 0) || (num_of_proc == 2 && process_id != 0 && process_id != 1))
    {
        fprintf(stderr, "error: process index out of bounds. process indexes start at 0\n");
        return '\0';
    }
    return pageFlowChart(process_id, address, LOAD_COMM_TYPE, '\0');
}
// helper functions:

/**
 * @brief gets a logcial address, converts it to a page and navigates the page to the correct place in order to use load or store
 *
 * @param process_id    - the index of the process to load/store info to/from. each process has its own page table.
 * @param address       - the logical address of the given process
 * @param commandType   - the type of command to execute (load = LOAD_COMM_TYPE, store = STORE_COMM_TYPE)
 * @param value         - the value to store in case the command is a store command
 * @return              - the char in the physical memory if the command is a load command
 */
char sim_mem::pageFlowChart(int process_id, int address, int commandType, char value)
{
    int page = getPageFromAddress(address, this->page_size, this->num_of_pages);
    // return -1 and print error if page is not in page_table
    if (page == -1)
    {
        fprintf(stderr, "error: invalid address, address %d does not belong to any page\n", address);
        return '\0';
    }
    // if page in main memory already, get physical address and return the char at physical address
    if (this->page_table[process_id][page].V == 1)
    {
        return executeCommand(process_id, address, page, commandType, value);
    }
    else
    {
        // page not in main memory

        // no write permissions, then page must be a text file (from the executable)
        if (this->page_table[process_id][page].P == 0)
        {
            // copy from exe file

            // read relevant page from executable (text) into toInsert
            char toInsert[page_size + 1];
            lseek(program_fd[process_id], page * page_size, SEEK_SET);
            read(program_fd[process_id], toInsert, page_size);
            toInsert[page_size] = '\0';

            // copy to main mem and return the info
            copyToMainMem(page, process_id, toInsert);
            return executeCommand(process_id, address, page, commandType, value);
        }
        else
        {
            // we can write to the page, so check if page is dirty
            if (page_table[process_id][page].D == 1)
            {
                // bring page from swap to main mem
                char toInsert[page_size];
                lseek(swapfile_fd, page_table[process_id][page].swap_index, SEEK_SET);
                read(swapfile_fd, toInsert, page_size);

                // delete page from swap
                deleteFromSwap(page, process_id);

                // copy page to main mem and execute commandType
                copyToMainMem(page, process_id, toInsert);
                return executeCommand(process_id, address, page, commandType, value);
            }
            else
            {
                // page could either be bss/stack/heap, or data
                // memory hierarchy: text -> data -> bss -> heap/stack
                // if data:
                if (page >= num_of_text_pages && page < num_of_text_pages + num_of_data_pages)
                {
                    // text and data are inside the exec file, so retreive data from exe file

                    // if already in swap, bring it from the swap:
                    char toInsert[page_size + 1];

                    if (page_table[process_id][page].swap_index != -1)
                    {
                        lseek(swapfile_fd, page_table[process_id][page].swap_index, SEEK_SET);
                        read(swapfile_fd, toInsert, page_size);

                        // delete page from swap
                        deleteFromSwap(page, process_id);
                    }
                    // not in swap, so read the relevant page from executable into toInsert
                    else
                    {
                        lseek(program_fd[process_id], (num_of_text_pages * page_size) + ((page - num_of_text_pages) * page_size), SEEK_SET);
                        read(program_fd[process_id], toInsert, page_size);
                    }
                    toInsert[page_size] = '\0';

                    // copy to main_memory and update page table
                    copyToMainMem(page, process_id, toInsert);
                    return executeCommand(process_id, address, page, commandType, value);
                }
                // if not data then page must be bss/heap/stack
                else
                {
                    // create new mem filled with 0s
                    char toInsert[page_size + 1];
                    for (int i = 0; i < page_size; i++)
                    {
                        toInsert[i] = '0';
                    }
                    toInsert[page_size] = '\0';

                    // if page is in swap, bring it from the swap rather than allocating new memory for it
                    if (page_table[process_id][page].swap_index != -1)
                    {
                        lseek(swapfile_fd, page_table[process_id][page].swap_index, SEEK_SET);
                        read(swapfile_fd, toInsert, page_size);

                        // delete page from swap
                        deleteFromSwap(page, process_id);
                    }

                    // stack and heap pages cannot be loaded for the first time
                    // so first check if its a stack/heap page
                    if (page >= num_of_text_pages + num_of_data_pages + num_of_bss_pages &&
                        page < num_of_text_pages + num_of_data_pages + num_of_bss_pages + num_of_heapstack_pages)
                    {
                        // if V != 1 and D != 1 that means the page was never allocated, so we can't load it
                        if (commandType == LOAD_COMM_TYPE && page_table[process_id][page].V != 1 && page_table[process_id][page].D != 1)
                        {
                            fprintf(stderr, "error: cannot load heap / stack page before the first time it's been loaded\n");
                            return '\0';
                        }
                        // allocate new memory and write to it
                        else
                        {
                            copyToMainMem(page, process_id, toInsert);
                            return executeCommand(process_id, address, page, commandType, value);
                        }
                    }
                    // its a bss page
                    else
                    {
                        copyToMainMem(page, process_id, toInsert);
                        return executeCommand(process_id, address, page, commandType, value);
                    }
                }
            }
        }
    }
    return '\0';
}

/**
 * @brief executes either load or store functionality (either returning a value or setting a value in the physical memory)
 *
 * @param process_id    - the index of the process to load/store info to/from. each process has its own page table.
 * @param address       - the logical address of a memory
 * @param page          - the page of the page table of a given process
 * @param commandType   - either load or store, the type of command to execute (load = LOAD_COMM_TYPE, store = STORE_COMM_TYPE)
 * @param value         - the value to store in the physical memory in case commandType is a store command
 * @return              - the char in the physical memory if the command is a load command
 */
char sim_mem::executeCommand(int process_id, int address, int page, int commandType, int value)
{
    int physicalAddress = logicalMemToPhysical(process_id, address);
    if (physicalAddress <= -1)
    {
        fprintf(stderr, "error: invalid address, address %d does not belong to any page\n", address);
        return '\0';
    }
    if (commandType == STORE_COMM_TYPE)
    {
        // if page is a text page and command is store, print error as we can't write to it.
        if (page >= 0 && page < num_of_text_pages)
        {
            fprintf(stderr, "error: can't write to text pages\n");
            return '\0';
        }
        else
        {
            main_memory[physicalAddress] = value;
            page_table[process_id][page].D = 1;
        }
    }
    return main_memory[physicalAddress];
}

/**
 * @brief finds free space inside the swap file. "free space" consists of page_size bytes of 0s
 *
 * @return the index of available space in the swap file.
 */
int sim_mem::findSwapSpace()
{
    int result = -1;
    // iterate over every index in the swapfile that can
    // contain a page and check if any page lies there already
    for (int index = 0; index < num_of_proc * (page_size * (num_of_pages - num_of_text_pages)); index += page_size)
    {
        // check if index is in any of the swap indexes in page_table
        int isInPageTable = 0;
        for (int page = num_of_text_pages; page < num_of_pages; page++)
        {
            for (int procId = 0; procId < num_of_proc; procId++)
            {
                if (page_table[procId][page].swap_index == index)
                {
                    isInPageTable = 1;
                }
            }
        }
        if (isInPageTable == 0)
        {
            return index;
        }
    }
    return -1;
}

/**
 * @brief goes over the page_table of all processes and returns which page has the frame given.
 *
 * @param frame     - a given frame to search
 * @param procId    - the process index in which the frame was found
 * @return          - the resulted page index in which the frame was found
 */
int sim_mem::getPageFromFrame(int frame, int *procId)
{
    for (int proc = 0; proc < num_of_proc; proc++)
    {
        for (int page = 0; page < num_of_pages; page++)
        {
            if (page_table[proc][page].frame == frame)
            {
                *procId = proc;
                return page;
            }
        }
    }
    return -1;
}

/**
 * @brief brings a frame inside the physical memory to
 *  the first available space of size page_size inside the swap file.
 *
 */
void sim_mem::mainMemToSwap()
{
    // get swap index
    int swapIndex = findSwapSpace();
    if (swapIndex == -1)
    {
        // this should never happen as swap space was defined to have enough space to include all pages of both processes
        // if it happens anyway, scream.
        fprintf(stderr, "error: swap is full (no available space)\n");
        return;
    }

    // search what page in what process is holding the frame at main_memory[remove_ind]
    int procId = 0;
    int pageInMainMem = getPageFromFrame(remove_ind, &procId);

    // if pageInMainMem is not a text page, it can be stored in the swap file
    if (page_table[procId][pageInMainMem].P == 1)
    {
        // memory is full -> dump a page at remove_ind to the swap file
        char toDump[page_size];
        strncpy(toDump, main_memory + (remove_ind * page_size), page_size);

        // dump to swap and update page table
        lseek(swapfile_fd, swapIndex, SEEK_SET);
        write(swapfile_fd, toDump, page_size);

        page_table[procId][pageInMainMem].swap_index = swapIndex;
    }
    page_table[procId][pageInMainMem].frame = -1;
    page_table[procId][pageInMainMem].V = 0;
    num_of_valid_pages--;

    insert_ind = remove_ind;
    // advance remove_ind by 1
    remove_ind = (remove_ind + 1) % (MEMORY_SIZE / page_size);
}

/**
 * @brief copies a page of page_size bytes to the main memory (physical memory)
 *
 * @param pageNum   - the page index inside page_table
 * @param procId    - the id of the process trying to copy its memory to the main memory
 * @param toInsert  - the actual bytes to copy
 */
void sim_mem::copyToMainMem(int pageNum, int procId, char *toInsert)
{
    // if mem is full, bring a page to swap
    if (num_of_valid_pages == (MEMORY_SIZE / page_size))
    {
        // bring a page from main memory to the swap file
        mainMemToSwap();
    }
    // copy the page toInsert to main_memory[remove_ind]
    strncpy(main_memory + (insert_ind * page_size), toInsert, page_size);

    // update page table
    page_table[procId][pageNum].frame = insert_ind;
    page_table[procId][pageNum].swap_index = -1;
    page_table[procId][pageNum].V = 1;
    num_of_valid_pages++;

    insert_ind++;
}

/**
 * @brief deletes a page from the swap file (fills the space the page took with 0s)
 *
 * @param page      - the page that resides in the swap file to delete.
 * @param procId    - the id of the process that refers to the given page.
 */
void sim_mem::deleteFromSwap(int page, int procId)
{
    char temp[page_size];
    for (int i = 0; i < page_size; i++)
    {
        temp[i] = '0';
    }

    lseek(swapfile_fd, page_table[procId][page].swap_index, SEEK_SET);
    write(swapfile_fd, temp, page_size);

    page_table[procId][page].swap_index = -1;
}

/**
 * @brief Returns the page number from a given address.
 * Note that the method used is division.
 * In reality, if for example the page size was 5 and number of pages were 25,
 * that means we would need 8 bytes to represent the data inside that page, which will cause addresses to reach a max of 8 * 25 in size.
 *
 * By using division and modulo, addresses will reach a max of 5 * 25.
 *
 * All of this practically means that with page size of 5 and 25 number of pages, using this method we can only use addresses from 0 to 125.
 * When in reality we would need to use addresses from 0 to 200
 *
 * @param address       - the logical address to convert from
 * @param page_size     - the size in bytes of a page
 * @param num_of_pages  - the total number of pages inside the page_table for any process
 * @return              - the index of the corresponding page to the given address.
 */
int getPageFromAddress(int address, int page_size, int num_of_pages)
{
    int page = address / page_size;
    if (page < 0 || page >= num_of_pages)
    {
        return -1;
    }
    else
    {
        return page;
    }
}

/**
 * @brief gets a logical address and returns the frame in memory for that specific address
 *
 * the returned value is not actual index inside main_memory[],
 * but rather the index of the frame inside the main_memory[] (index * page_size = actual index inside main_memory[])
 *
 * @param processId         - the process to look for the right page/frame
 * @param logicalAddress    - the logical address
 * @return                  - the frame index
 */
int sim_mem::logicalMemToPhysical(int processId, int logicalAddress)
{
    int page = getPageFromAddress(logicalAddress, page_size, num_of_pages);
    int offset = logicalAddress % page_size;
    if (page == -1)
    {
        return -1;
    }

    int frame = page_table[processId][page].frame;

    return (frame * page_size) + offset;
}

#endif