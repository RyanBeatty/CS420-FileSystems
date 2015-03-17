# CS420-FileSystems
NACHOS File System implementation

------------------------------My implementation of Arbitrarily long files is as follows--------------------------------------------

############################################################
Classes added to implement Single and Double Indirect blocks
############################################################

from fileblock.cc

IndirectBlock:
		Class that represents a single indirect file block and implements an interface to allocate, remove, and calculate
	a byte offset for the file.

	Interface Summary:
		* An IndirectBlock object has an integer array called "dataSectors" which keeps track of allocated sector numbers on the disk.
		  "dataSectors" is MAX_BLOCKS size long where MAX_BLOCKS is the number of integers that can be stored in one disk sector.

		* IndirectBlock(): Creates the "dataSectors" array and initializes all of its values to be EMPTY_BLOCK where EMPTY_BLOCK
		  represents a block in the file that has not been allocated on the disk. EMPTY_BLOCK's value is (-1)

		* int Allocate(BitMap *, int): Allocate is the main interface for expanding or allocating space for files. Allocate() takes the
		  bitmap of the free sectors on the disk and the NUMBER OF SECTORS that need to be allocated. Allocate returns the number of
		  sectors that were allocated (if the number in the request is larger than MAX_BLOCKS) or (-1) signaling an error occurred.
		  The process for allocating space for file blocks is simple. Iterate over the "dataSectors" array. If the current element
		  is uninitialized (its value is EMPTY_BLOCK), then allocate it, else do nothing, meaning the element already contains the
		  sector number of an allocated block.

		* void Deallocate(BitMap *): Frees all of the allocated blocks in the IndirectBlock. Deallocate() takes the bitmap of the
		  free sectors on the disk as a parameter. Iterates over the "dataSectors" array. If an element is not the EMPTY_BLOCK
		  (the block has been allocate), free the block, else do nothing.

		* WriteBack(int): writes the "dataSectors" array to disk

		* FetchFrom(int): reads the "dataSectors" array from disk

		* int ByteToSector(int)











------------------------TESTING-----------------------------------------


############
share1.c
sharekid1.c
############

share1.c tests that child processes inherit the same open files and file descriptors as
ther parent process. share1.c begins by reading in 20 characters from the file "data" (which contains the letters a-z)
and then forks/execs sharekid1 which simply reads one more character, prints the character, and exits. The parent
process waits for the child to exit before reading one more character.

files to cp over:
/test/data
/test/share1
/test/sharekid1

run:
./nachos -x share1

***Output****
Output Open returned descriptor 2
PARENT read 20 bytes
Data from the read was: <abcdefghijklmnopqrst>
KID about to read from inherited file
KID read 1 bytes
Data from the read was: <u>
KID read from closed file returned -1
PARENT off Join with value of 17
PARENT about to read a byte from shared file
PARENT read 1 bytes
Data from the read was: <v>
PARENT read from closed file returned -1



###########
share2.c
sharekid2.c
###########

share2.c tests that concurrent processes that open the same file recieve seperate, private seek positions
within the file. share2.c begins by reading and printing 20 characters from the file "data" and then forks/execs
sharekid2.c which opens the file "data" and reads and prints 10 characters (which should be the first 10 
characters in the file) before exiting. share2.c waits for the child to exit and reads and prints one more 
character (which should be the 21 character from the file) before exiting.

files to cp over:
/test/data
/test/share2
/test/sharekid2

run:
./nachos -x share2

****Output****
Output Open returned descriptor 2
PARENT read 20 bytes
Data from the read was: <abcdefghijklmnopqrst>
KID Output Open returned descriptor 3
KID read 10 bytes
Data from the read was: <abcdefghij>
KID read from closed file returned -1
PARENT off Join with value of 17
PARENT about to read a byte from shared file
PARENT read 1 bytes
Data from the read was: <u>
PARENT read from closed file returned -1
