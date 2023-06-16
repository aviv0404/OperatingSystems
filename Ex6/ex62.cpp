/*
ID: 211313333
Name: Aviv Zvuluny

Hard Drive Simulation - Ex 6

Description:
    - This program simulates how the operating system would save information inside files using the indexed allocation method

    - The user can create a file, delete a file, open a file, close a file, write to a file, read from a file
      and list all the files and the entire contents of the disk (including index blocks).

    - The indexed allocation method is a method where each file holds a block of pointers to other blocks where these blocks hold information.
      This means that the max file size of every single file in our system is the block size squared.

Assumptions/Notes:
    - The file DISK_SIM_FILE.txt must exist inside the directory of the main cpp file.
      Otherwise, an error will be thrown and the program won't start

    - I've modified listAll() to print index blocks as numbers so weird characters don't screw up the print
*/

#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define DISK_SIZE 256

// ============================================================================

/**
 * @brief casts a number to a char.
 * for example, if n = 5 then c will be equal to '\005'
 * 
 * @param n the number to conver to a char
 * @param c a reference to a char, the result will be stored here
 */
void decToBinary(int n, char &c)
{
    // array to store binary number
    int binaryNum[8];

    // counter for binary array
    int i = 0;
    while (n > 0)
    {
        // storing remainder in binary array
        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }

    // printing binary array in reverse order
    for (int j = i - 1; j >= 0; j--)
    {
        if (binaryNum[j] == 1)
            c = c | 1u << j;
    }
}

// ============================================================================

/**
 * @brief This class is responsible for managing the blocks for a file.
 */
class FsFile
{
    int file_size;
    int index_block;
    int block_size;
    int block_in_use; // how many blocks this file uses, including the index block

public:
    FsFile(int _block_size)
    {
        file_size = 0;
        block_in_use = 0;
        block_size = _block_size;
        index_block = -1;
    }

    FsFile(int _block_size, int _index_block)
    {
        file_size = 0;
        block_in_use = 1;
        block_size = _block_size;
        index_block = _index_block;
    }

    // Getters
    int getfile_size()
    {
        return file_size;
    }

    int get_index_block()
    {
        return index_block;
    }

    int get_block_in_use()
    {
        return block_in_use;
    }

    // Setters
    void set_file_size(int _file_size)
    {
        file_size = _file_size;
    }

    // opens a pointer to a new block
    void openNewPointerToBlock(int *&BitVector, int BitVectorSize, FILE *sim_disk_fd)
    {
        // index of the next pointer block
        int index_inside_sim_file = index_block * block_size + (int)(file_size / block_size);
        for (int i = 0; i < BitVectorSize; i++)
        {
            if (BitVector[i] == 0)
            {
                // write the index of the free block to index_inside_sim_file which is the pointer to this new block
                fseek(sim_disk_fd, index_inside_sim_file, SEEK_SET);
                char indexAsChar = '\0';
                decToBinary(i, indexAsChar);
                fwrite(&indexAsChar, sizeof(indexAsChar), 1, sim_disk_fd);

                BitVector[i] = 1;
                block_in_use++;
                break;
            }
        }
    }

    void releaseAllBlocks()
    {
        block_in_use = 0;
    }
};

// ============================================================================

/**
 * @brief a class that links between an FsFile and its name
 */
class FileDescriptor
{
    string file_name;
    bool inUse;
    FsFile *fs_file;

public:
    FileDescriptor(string FileName, FsFile *fsi)
    {
        file_name = FileName;
        fs_file = fsi;
        inUse = true;
    }

    string getFileName()
    {
        return file_name;
    }

    FsFile *getFsFile()
    {
        return fs_file;
    }
};

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"

/**
 * @brief Manages the disk using the indexed allocation format method.
 * 
 */
class fsDisk
{
private:
    FILE *sim_disk_fd;
    bool is_formated;

    // BitVector - "bit" (int) vector, indicate which block in the disk is free
    //              or not.  (i.e. if BitVector[0] == 1 , means that the
    //             first block is occupied.
    int BitVectorSize;
    int *BitVector;

    // A map that links the file descriptor number to its FileDescriptor class.
    // This way each file has a unique file descriptor number
    map<int, FileDescriptor> MainDir;

    // A map that holds the opened files in the system. if a file is inside this map then it is used.
    // This map links a file descriptor number to its FileDescriptor class
    map<int, FileDescriptor> OpenFileDescriptors;

    int block_size;

    /**
     * @brief attempts to find a given file inside the main directory
     *
     * @param fd the file descriptor number of the file to find
     * @param fileName the file name of the file to find
     * @return true if file was found, false otherwise
     */
    bool fileExists(int fd = -1, string fileName = "")
    {
        for (pair<int, FileDescriptor> element : MainDir)
        {
            if (element.first == fd || element.second.getFileName().compare(fileName) == 0)
            {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief loops over OpenFileDescriptors and attempts to find a given file name or file descriptor in there.
     *
     * @param fd a given file descriptor number of the file we want to search for
     * @param fileName a given filename of the file we want to search for
     * @return true if file was found, false otherwise.
     */
    bool isFileOpen(int fd = -1, string fileName = "")
    {
        bool result = false;
        for (pair<int, FileDescriptor> element : OpenFileDescriptors)
        {
            if (element.first == fd || element.second.getFileName().compare(fileName) == 0)
            {
                result = true;
            }
        }
        return result;
    }

    /**
     * @return the index of said block inside the BitVector, and -1 if there is no free block.
     */
    int findFreeBlock()
    {
        for (int i = 0; i < BitVectorSize; i++)
        {
            if (BitVector[i] == 0)
            {
                return i;
            }
        }

        return -1;
    }

    /**
     * @brief For a given file, this function finds the index inside DISK_SIM_FILE.txt
     *  of the next character we can write to.
     * (as we're formatting using indexed allocation, we need to find the next block which we can still write to, and then find the next index)
     *
     * This method cannot fail.
     *
     * @param file the given file to find the next index for
     * @return the index of the next available char
     */
    int getIndexOfCurrentBlock(FsFile *file)
    {
        // block_num = the block we can currently write to (values are: 0,1,2,3)
        int block_num = (int)((*file).getfile_size() / block_size);

        // get current block from sim_file
        int temp = ((*file).get_index_block() * block_size) + block_num;
        fseek(sim_disk_fd, temp, SEEK_SET);
        char indexAsChar = '\0';
        fread(&indexAsChar, sizeof(indexAsChar), 1, sim_disk_fd);

        int current_block = (int)indexAsChar;
        //---------------- get the index of the block we can write to ----------------
        int offset_inside_block = (*file).getfile_size() % block_size;
        return (current_block * block_size) + offset_inside_block;
    }

    /**
     * @brief counts the number of free blocks (number of blocks that no file holds)
     *
     * @return the number of free blocks
     */
    int numOfFreeBlocks()
    {
        int result = 0;
        for (int i = 0; i < BitVectorSize; i++)
        {
            if (BitVector[i] == 0)
            {
                result++;
            }
        }
        return result;
    }

public:
    fsDisk()
    {
        sim_disk_fd = fopen(DISK_SIM_FILE, "r+");
        assert(sim_disk_fd);

        for (int i = 0; i < DISK_SIZE; i++)
        {
            int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fwrite("\0", 1, 1, sim_disk_fd);
            assert(ret_val == 1);
        }

        fflush(sim_disk_fd);
        is_formated = false;
    }

    ~fsDisk()
    {
        free(BitVector);
    }

    /**
     * @brief prints all the files inside the main directory as well as all the content of the disk (DISK_SIM_FILE.txt)
     *
     * note that blocks inside an index block will be converted to their
     * number varient as a string to avoid printing weird characters that might mess up the entire print
     * (in some systems some characters contain multiple new lines inside them, and some of them even eat up previous characters)
     *
     */
    void listAll()
    {
        int i = 0;

        for (pair<int, FileDescriptor> element : MainDir)
        {
            cout << "index: " << element.first << ": FileName: " << element.second.getFileName()
                 << " , isInUse: "
                 << isFileOpen(element.first) << endl;
            i++;
        }

        char bufy;
        cout << "Disk content: '";
        for (i = 0; i < DISK_SIZE; i++)
        {
            cout << "(";
            int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fread(&bufy, 1, 1, sim_disk_fd);

            // check if i is inside an index block. if so, raise a flag so we
            // can print the number version of said char rather than
            // printing the char itself which has a chance to look weird and screw
            // the print (since index blocks are numbers converted to chars).
            bool indexBlockPrintFlag = false;
            for (pair<int, FileDescriptor> element : MainDir)
            {
                int index_block = element.second.getFsFile()->get_index_block();
                // if i is inside an index block
                if (i >= index_block * block_size && i < (index_block + 1) * block_size)
                {
                    indexBlockPrintFlag = true;
                    break;
                }
            }
            if (indexBlockPrintFlag && bufy != '\0')
            {

                cout << (int)bufy;
            }
            else
            {
                cout << bufy;
            }
            cout << ")";
        }
        cout << "'" << endl;
    }

    /**
     * @brief formats the disk using the indexed allocation method. in this method each file can hold a
     * max size of (blockSize * blockSize) chars, and holds an additional block called "index block",
     * where each char in this block is a pointer to a block of blockSize chars of information inside the file.
     *
     * @param blockSize the block size of all the files in the system
     */
    void fsFormat(int blockSize = 4)
    {
        if (!is_formated)
        {
            // init bit vector and set block size
            BitVectorSize = DISK_SIZE / blockSize;
            BitVector = (int *)malloc(sizeof(int) * BitVectorSize);
            block_size = blockSize;
            is_formated = true;
        }
        else
        {
            // new format means we can't use the old format, so clean everything
            // print confirmation to the user
            cout << "Formatting this disk will delete all its content, are you sure? [Y/N]: ";
            string yn;
            cin >> yn;
            while (yn.compare("Y") != 0 && yn.compare("y") != 0 && yn.compare("N") != 0 && yn.compare("n") != 0)
            {
                cout << "Please enter Y or N: ";
                cin >> yn;
            }
            if (yn.compare("Y") == 0 || yn.compare("y") == 0)
            {
                // set disk to '\0's
                for (int i = 0; i < DISK_SIZE; i++)
                {
                    fseek(sim_disk_fd, i, SEEK_SET);
                    fwrite("\0", sizeof(char), 1, sim_disk_fd);
                }

                // free previous bit vector and init a new one
                free(BitVector);
                BitVectorSize = DISK_SIZE / blockSize;
                BitVector = (int *)malloc(sizeof(int) * BitVectorSize);
                block_size = blockSize;

                // delete all files that were created previously
                MainDir.clear();
                OpenFileDescriptors.clear();
            }
            else
            {
                fprintf(stderr, "Format request aborted\n");
                return;
            }
        }
        for (int i = 0; i < BitVectorSize; i++)
        {
            BitVector[i] = 0;
        }
    }

    /**
     * @brief creates a new file (allocates an index block for it, and adds it to the main directory)
     *
     * @param fileName the name of the file to create
     * @return -1 on failure, and the file descriptor number of the new file on success.
     */
    int CreateFile(string fileName)
    {

        if (!is_formated)
        {
            fprintf(stderr, "disk needs to be formatted before creating a file\n");
            return -1;
        }
        if (fileExists(-1, fileName))
        {
            fprintf(stderr, "file %s already exists\n", fileName.c_str());
            return -1;
        }
        else
        {
            int index_block = findFreeBlock();
            if (index_block == -1)
            {
                fprintf(stderr, "disk is full, cannot create a new file\n");
                return -1;
            }
            BitVector[index_block] = 1;

            FsFile *ff = new FsFile(block_size, index_block);
            FileDescriptor fd(fileName, ff);
            // associate each FileDescriptor object with a unique file descriptor number
            int fdNum = 0;
            // loop over all previous keys, just in case a file was deleted
            for (int i = 0; i <= MainDir.size(); i++)
            {
                if (MainDir.count(i) == 0)
                {
                    fdNum = i;
                    break;
                }
            }
            MainDir.insert(make_pair(fdNum, fd));
            OpenFileDescriptors.insert(make_pair(fdNum, fd));
            return fdNum;
        }
    }

    /**
     * @brief opens a file (opens up the options to write to / read from it and blocks the option to delete it)
     *
     * @param fileName the name of hte file to open
     * @return -1 on failure, and the file descriptor number of the file on success
     */
    int OpenFile(string fileName)
    {

        // if file is opened already, return -1 and print an error:
        for (pair<int, FileDescriptor> element : OpenFileDescriptors)
        {
            if (element.second.getFileName().compare(fileName) == 0)
            {
                fprintf(stderr, "file %s is already open\n", fileName.c_str());
                return -1;
            }
        }

        // if file exists, add it to the OpenFileDescriptors list
        for (pair<int, FileDescriptor> element : MainDir)
        {
            if (element.second.getFileName().compare(fileName) == 0)
            {
                OpenFileDescriptors.insert(make_pair(element.first, element.second));
                return element.first;
            }
        }

        // file doesn't exist, print error and return -1
        fprintf(stderr, "file %s does not exist\n", fileName.c_str());
        return -1;
    }

    /**
     * @brief closes a file (opens up the options to delete it and blocks the options to write to / read from it)
     *
     * @param fd the file descriptor number of the file to close
     * @return the literal string "-1" on failure, and the name of the file on success
     */
    string CloseFile(int fd)
    {

        if (fileExists(fd) && isFileOpen(fd))
        {
            string temp = OpenFileDescriptors.find(fd)->second.getFileName();
            OpenFileDescriptors.erase(fd);
            return temp;
        }
        else
        {
            fprintf(stderr, "file #%d does not exist or is already open\n", fd);
            return "-1";
        }
    }

    /**
     * @brief writes a given string to the file using the indexed allocation method
     *
     * @param fd the file descriptor number of the file to write to
     * @param buf the string to write
     * @param len length of the string to write (not including null character)
     * @return -1 if an error occurred while trying to write, 1 otherwise
     */
    int WriteToFile(int fd, char *buf, int len)
    {
        if (!fileExists(fd))
        {
            fprintf(stderr, "file #%d doesn't exist\n", fd);
            return -1;
        }
        else
        {
            if (!isFileOpen(fd))
            {
                fprintf(stderr, "file #%d is not open\n", fd);
                return -1;
            }
            else
            {
                if (!is_formated)
                {
                    fprintf(stderr, "disk needs to be formatted before writing to files\n");
                    return -1;
                }
                FsFile *file = OpenFileDescriptors.find(fd)->second.getFsFile();

                if ((block_size * block_size) - file->getfile_size() < len)
                {
                    fprintf(stderr, "cannot write to %s. writing the given buffer will exceed max file size (%d chars)\n",
                            OpenFileDescriptors.find(fd)->second.getFileName().c_str(), block_size * block_size);
                    return -1;
                }

                int numOfBlocksToAllocate = ceil(len / (double)block_size);
                if (findFreeBlock() == -1 || (numOfFreeBlocks() - numOfBlocksToAllocate) < 0)
                {
                    fprintf(stderr, "writing %s will exceed max disk space. please free up some space before attempting to write to a file.\n", buf);
                    return -1;
                }

                // write to the first available block in file
                // if block is full, open a new block and keep writing
                for (int i = 0; i < len; i++)
                {
                    // if current block is full
                    if ((*file).getfile_size() % block_size == 0)
                    {
                        (*file).openNewPointerToBlock(BitVector, BitVectorSize, sim_disk_fd);
                    }

                    int index_inside_block = getIndexOfCurrentBlock(file);

                    fseek(sim_disk_fd, index_inside_block, SEEK_SET);
                    fwrite(&buf[i], sizeof(char), 1, sim_disk_fd);

                    // increase file size
                    file->set_file_size(file->getfile_size() + 1);
                }
                return 1;
            }
        }
    }

    /**
     * @brief deletes a file from the main directory. also deletes all its content in DISK_SIM_FILE.txt,
     * including the index block. (assumes the file needs to be closed before deleting it)
     *
     * @param FileName the file to delete
     * @return -1 if the operation was not successful, and the file descriptor number if it was successful
     */
    int DelFile(string FileName)
    {
        if (!fileExists(-1, FileName))
        {
            fprintf(stderr, "file %s doesn't exist.\n", FileName.c_str());
            return -1;
        }
        if (isFileOpen(-1, FileName))
        {
            fprintf(stderr, "file %s is open, please close this file before deleting it.\n", FileName.c_str());
            return -1;
        }
        // get a pointer to FsFile
        FsFile *ff = NULL;
        int fd;
        for (pair<int, FileDescriptor> element : MainDir)
        {
            if (element.second.getFileName().compare(FileName) == 0)
            {
                ff = element.second.getFsFile();
                fd = element.first;
            }
        }
        // for each pointer block
        for (int i = 0; i < ff->get_block_in_use() - 1; i++)
        {
            // get index of block
            char indexAsChar;
            fseek(sim_disk_fd, (ff->get_index_block() * block_size) + i, SEEK_SET);
            fread(&indexAsChar, sizeof(indexAsChar), 1, sim_disk_fd);

            // clear current block from BitVector
            BitVector[(int)indexAsChar] = 0;

            // clean block
            for (int j = 0; j < block_size; j++)
            {
                // then get to the block and delete its content
                fseek(sim_disk_fd, (int)indexAsChar * block_size + j, SEEK_SET);
                char temp = '\0';
                fwrite(&temp, sizeof(temp), 1, sim_disk_fd);
            }

            // delete pointer
            fseek(sim_disk_fd, (ff->get_index_block() * block_size) + i, SEEK_SET);
            char temp = '\0';
            fwrite(&temp, sizeof(temp), 1, sim_disk_fd);
        }

        // clear index block from BitVector
        BitVector[ff->get_index_block()] = 0;

        // delete file from MainDir and OpenFileDescriptors
        MainDir.erase(fd);
        OpenFileDescriptors.erase(fd);
        return fd;
    }

    /**
     * @brief reads len characters from a given file.
     * (assumes len is the requested number of characters to read from the file, and the size of buf is len+1)
     *
     * @param fd the file descriptor number to read from
     * @param buf the string to save the file contents to
     * @param len the requested num of characters to read from the file
     * @return -1 if an error has occurred, and 0 otherwise
     */
    int ReadFromFile(int fd, char *buf, int len)
    {
        if (!fileExists(fd))
        {
            fprintf(stderr, "file #%d does not exist\n", fd);
            return -1;
        }
        if (!isFileOpen(fd))
        {
            fprintf(stderr, "file #%d is not open. a file needs to be open in order to read from it\n", fd);
            return -1;
        }
        if (!is_formated)
        {
            fprintf(stderr, "disk needs to be formatted before reading from it\n");
            return -1;
        }

        int count = 0; // how many chars were read
        FsFile *ff = OpenFileDescriptors.find(fd)->second.getFsFile();
        if (ff->getfile_size() < len)
        {
            fprintf(stderr, "cannot read more than the file contains (%d chars currently)\n", ff->getfile_size());
            return -1;
        }
        char indexAsChar;
        // for each pointer block
        for (int i = 0; i < ff->get_block_in_use() - 1; i++)
        {

            int pointer_to_block = (ff->get_index_block() * block_size) + i;
            fseek(sim_disk_fd, pointer_to_block, SEEK_SET);
            fread(&indexAsChar, sizeof(indexAsChar), 1, sim_disk_fd);

            // determine how many chars to read from current block
            int toReadFromBlock = (ff->getfile_size() / block_size > 0) ? block_size : ff->getfile_size();
            int offsetInsideBlock = 0;
            // read from current block
            while (count < len && toReadFromBlock > 0)
            {
                char temp;
                fseek(sim_disk_fd, (int)indexAsChar * block_size + offsetInsideBlock, SEEK_SET);
                fread(&temp, sizeof(temp), 1, sim_disk_fd);
                buf[count] = temp;
                offsetInsideBlock++;
                count++;
                toReadFromBlock--;
            }
        }
        buf[count] = '\0';
        return 0;
    }
};

// ============================================================================

int main()
{
    int blockSize;
    int direct_entries;
    string fileName;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read;
    int _fd;

    fsDisk *fs = new fsDisk();
    int cmd_;
    while (1)
    {
        cout << "0: Exit\n"
             << "1: List All\n"
             << "2: Format\n"
             << "3: Create File\n"
             << "4: Open File\n"
             << "5: Close File\n"
             << "6: Write to File\n"
             << "7: Read From File\n"
             << "8: Delete File\n"
             << endl;
        cin >> cmd_;
        switch (cmd_)
        {
        case 0: // exit
            delete fs;
            exit(0);
            break;

        case 1: // list-file
            fs->listAll();
            break;

        case 2: // format
            cout << "Block size: ";
            cin >> blockSize;
            fs->fsFormat(blockSize);
            cout << "FORMAT DISK: number of blocks: " << DISK_SIZE / blockSize << endl;
            break;

        case 3: // create-file
            cout << "File name: ";
            cin >> fileName;
            _fd = fs->CreateFile(fileName);
            cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

        case 4: // open-file
            cout << "File name: ";
            cin >> fileName;
            _fd = fs->OpenFile(fileName);
            cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

        case 5: // close-file
            cout << "File Descriptor number: ";
            cin >> _fd;
            fileName = fs->CloseFile(_fd);
            cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

        case 6: // write-file
            cout << "File Descriptor number: ";
            cin >> _fd;
            cout << "String to write: ";
            cin >> str_to_write;
            fs->WriteToFile(_fd, str_to_write, strlen(str_to_write));
            break;

        case 7: // read-file
            cout << "File Descriptor number: ";
            cin >> _fd;
            cout << "Size to read: ";
            cin >> size_to_read;
            fs->ReadFromFile(_fd, str_to_read, size_to_read);
            cout << "ReadFromFile: " << str_to_read << endl;
            break;

        case 8: // delete file
            cout << "File name: ";
            cin >> fileName;
            _fd = fs->DelFile(fileName);
            cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;
        default:
            break;
        }
    }
}