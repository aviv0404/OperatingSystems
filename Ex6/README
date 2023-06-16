ID: 211313333
Name: Aviv Zvuluny

Hard Drive Simulation - Ex 6

Description:
    - This program simulates how the operating system would save information inside files using the indexed allocation method

    - The user can create a file, delete a file, open a file, close a file, write to a file, read from a file
      and list all the files and the entire contents of the disk (including index blocks).

    - The indexed allocation method is a method where each file holds a block of pointers to other blocks where these blocks hold information.
      This means that the max file size of every single file in our system is the block size squared.


Program DATABASE:
    - The main ways to store information in this program are:
        1) map<int, FileDescriptor> MainDir, OpenFileDescriptor: a map that links each FileDescriptor to a 
                                                                 unique number thats associated with it.
        2) FsFile: a class that holds information about the blocks of a file
        3) FileDescriptor: a class that links a file name to its FsFile class

    - files:
        DISK_SIM_FILE.txt: this is a text file that simulates our disk. it contains characters 
                              that hold information about the files in our system as well as their content.

Assumptions/Notes:
    - The file DISK_SIM_FILE.txt must exist inside the directory of the main cpp file.
      Otherwise, an error will be thrown and the program won't start

    - I've modified listAll() to print index blocks as numbers so weird characters don't screw up the print

    - The index you see in listAll does not mean the file descriptor, 
      you have to keep track of your own file descriptors of the files you open.

    - MainDir indexes don't update when a file is deleted, but will be filled up when a new file is created.

Functions:
    - void listAll() - Prints all the files in the main directory as well as all the content inside DISK_SIM_FILE.txt.

    - void fsFormat(int blockSize = 4) - Formats the disk using the indexed allocation method.

    - int CreateFile(string fileName) - Creates a new file using the indexed allocation method.

    - int OpenFile(string fileName) - Opens a file for reading and writing.

    - string CloseFile(int fd) - Closes a file.

    - int WriteToFile(int fd, char *buf, int len) - Writes the given string to a file if possible.

    - int ReadFromFile(int fd, char *buf, int len) - Reads len chars from the given file to the char array buf.

    - int DelFile(string FileName) - Deletes a file from the main directory as well as all its blocks and its index block inside the disk.


Program Files:
    - ex6.cpp (includes the main function)

compile and run (Linux):
    - compile: g++ ex6.cpp -o ex6
    - run: ./ex6

Input:
    - Given a number the user can decide which function to execute.

Output:
    - The appropriate output for the function the user chose.
      example: input: 2 -> 4 
               output: "FORMAT DISK: number of blocks 64"