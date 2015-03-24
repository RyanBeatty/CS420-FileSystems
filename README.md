# CS420-FileSystems
NACHOS File System implementation

----My implementation of Arbitrarily long files is as follows----

############################################################
Classes added to implement Single and Double Indirect blocks
############################################################

from fileblock.cc

#############
IndirectBlock
#############

* Class that represents a single indirect file block and implements an interface to allocate, remove, and calculate a byte offset for the file.

* An IndirectBlock object has an integer array called "dataSectors" which keeps track ofallocated sector numbers on the disk. "dataSectors" is MAX_BLOCKS size long where MAX_BLOCKS is the number of integers that can be stored in one disk sector.

* IndirectBlock(): Creates the "dataSectors" array and initializes all of its values to beEMPTY_BLOCK where EMPTY_BLOCK represents a block in the file that has not been allocated on the disk. EMPTY_BLOCK's value is (-1)

* int Allocate(BitMap *, int): Allocate is the main interface for expanding or allocating space for files. Allocate() takes the bitmap of the free sectors on the disk and the NUMBER OF SECTORS that need to be allocated. Allocate returns the number of sectors that were allocated (if the number in the request is larger than MAX_BLOCKS) or (-1) signaling an error occurred. The process for allocating space for file blocks is simple. Iterate over the "dataSectors" array. If the current element is uninitialized (its value is EMPTY_BLOCK), then allocate it, else do nothing, meaning the element already contains the sector number of an allocated block.

* void Deallocate(BitMap *): Frees all of the allocated blocks in the IndirectBlock. Deallocate() takes the bitmap of the free sectors on the disk as a parameter. Iterates over the "dataSectors" array. If an element is not the EMPTY_BLOCK (the block has been allocate), free the block, else do nothing.

* WriteBack(int): writes the "dataSectors" array to disk

* FetchFrom(int): reads the "dataSectors" array from disk

* int ByteToSector(int): takes in an integer representing the byte offset in the file that needs to be accessed and returns the corresponding sector where the offset is stored.

###################
DoublyIndirectBlock
###################

* Class that represents a doubly indirect file block and implements an interface to allocate, remove, and calculate a byte offset for the file. A doubly indirect block stores data sectors which contain indirect blocks which point to actual file blocks on the disk.

* "dataSectors": DoublyIndirectBlock objects have an integer array called "dataSectors" which stores sector numbers where IndirectBlock objects are stored on the disk.

* DoublyIndirectBlock(): creates and initializes every element of the "dataSectors" array to be EMPTY_BLOCK, meaning the element hasn't been allocated on the disk yet.

* int Allocate(BitMap *, int): Allocate is the main interface for intializing, expanding, or allocating more space for files. Allocate() takes in the bitmap of the free sectors on the disk and the number of sectors that needs to be allocated as parameters and returns the number of sectors that were actually allocated or (-1) if there was an error. Method of allocation is as follows: iterate over the "dataSectors" array, allocating sectors across IndirectBlock objects until the requested number of sectors has been allocated. If the current element has not been allocated on the disk, create a new IndirectBlock object and allocate it on the disk, subtracting the number of sectors the new IndirectBlock allocated from the request total. If the current element has already been allocated on the disk, fetch the IndirectBlock object from disk and try to allocate as many sectors as possible.

* void Deallocate(BitMap *): Deallocates every IndirectBlock object that has been allocated in this instance of a DoublyIndirectBlock object. Deallocate() takes the bitmap of the free sectors on the disk as parameters. Iterate over the "dataSectors" array, load up the allocated IndirectBlock objects from disk, and call each IndirectBlock object's Deallocate() method.

* WriteBack(int): writes the "dataSectors" array to disk

* FetchFrom(int): reads the "dataSectors" array from disk

* int ByteToSector(int): Returns the sector on the disk that a given byte offset into a file is stored at. First calculates the logical block that the offset is at, indexes into the "dataSectors" array to find the sector number of the IndirectBlock object that contains the physical sector the logical block is stored at, fetches the IndirectBlock object from the disk, and then calls the IndirectBlock object's ByteToSector() method to get the sector number of where the offset into the file is stored on disk.


###################################################
Changes made to FileHeader to implement large files  
###################################################

* FileHeader objects are now created with a constructor that intializes "numBytes" and "numSectors" to be 0 and all elements of "dataSectors" to be EMPTY_BLOCKS.

* The "dataSectors" array now holds sector locations on the disk of allocated DoublyIndirectBlock objets.

* Allocate() now iterates over the "dataSectors" array and allocates DoublyIndirectBlock objects on the disk instead of direct blocks in order to allow arbitrarily long files to be stored on disk. When allocating a new DoublyIndirectBlock, simply create a new DoublyIndirectBlock object and then call the object's Allocate() method passing in the number of sectors that need to be allocated.

* Deallocate() now deallocates all of the DoublyIndirectBlock objects that have been allocated by the FileHeader object. When iterating over the "dataSectors" array, fetch the allocated DoublyIndirectBlock objects that have been allocated from the disk and then simply call the object's Deallocate() method.

* ByteToSector() now calculates which DoublyIndirectBlock contains the sector where the offset is stored, loads up the corresponding DoublyIndirectBlock object from the disk, and then calls ByteToSector() on the DoublyIndirectBlock object to get the corresponding sector where the offset is stored on disk.



----My implementation of extendable files is as follows----

########################################################
Changes made to FileHeader to implement extendable files
########################################################

* Allocate() can now be used to allocate more space for a file at any point during the FileHeader object's lifetime (previously it could only be used when first allocating a FileHeader object). Allocate iterates over the "dataSectors" array and keeps allocating space until, the allocation request has been served fully. The DoublyIndirectBlock class's Allocate() method returns the amount of sectors allocated, so it is easy for the FileHeader to keep track of how much space still needs to be allocated for the request.

* Allocate() also adds the number of requested bytes and sectors to be allocated to the "numSectors" and "numBytes"


######################################################
Changes made to OpenFile to implement extendable files
######################################################

in openfile.h

* Added new private integer variable "hdrSector" which keeps track of the sector on the disk where the FileHeader object is stored.


in openfile.cc

* The OpenFile() constructor now sets "hdrSector" to be the passed in sector argument to the constructor

* Write() now checks to see if we are writing off the end of the file and resizes the file. First check if the current seek position plus the number of bytes we wish to write to the file is greater than the file lenght in bytes. If it is, resize the file, else we just service the write.

* When resizing files, first create a new BitMap and fetch the disk bitmap from disk. Then allocate enough space to service the write request by calling Allocate() on the OpenFile's "hdr" FileHeader object. Then write "hdr" and the modified disk bitmap back to disk.


################################################################
Changes made to Directory to implement extendable directory size
################################################################

in directory.h

* Added a new method, Expand(), for expanding the directory

* "INCREASE_FACTOR": macro which specifies the factor by which the directory object should grow when expanding. Currently set so that the directory will double in size for each Expand().

in directory.cc

* void Expand(int): new method that takes in the new directory size as a parameter. It creates a new DirectoryEntry array and copies over all of the entries in the current "table" array and then assigns "table" to point to the new table and "tableSize" to be the passed in size.

* Add() now checks if the directory needs to be resized. If when iterating over the "table" array, there is no more space, resize the table and pick the first open slot to store the new DirectoryEntry at.

* Fetching/Writing the Directory object to/from disk. Since the Directory no longer has once static, set size, the size of the table needs to be written to disk in order to be able to be able to recreate the Directory object when fetching it from the disk. Also, since FetchFrom/WriteBack used ReadAt/WriteAt, I had to change them to use OpenFile::Read() and OpenFile::Write() so that the Directory file can expand when the directory object expands.

* WriteBack() now writes the "tableSize" to disk along with the "table" array. First call Seek(0) on the passed in OpenFile object just to make sure we are going to write data starting at the beginning of the directory file. Then write the "tableSize" variable to the directory file (basically I cast the address returned by &tableSize to a (char *) and then write sizeof(int) bytes). Then write the table to the directory file, making sure to write "tableSize" * sizeof(DirectoryEntry) bytes. Finally, move the seekposition back to the start of the directory file by calling Seek(0) on the OpenFile object.

* FetchFrom() now takes into acount directory expansion. First call Seek(0) on the passed in directory OpenFile object in order to make sure we will be reading data from the beginning of the directory file. Then read the "tableSize" value from the file and store it in an integer (basically read sizeof(int) bytes of data from the file). If the size read is greater than the size the Directory object was initialized with, call Expand() and pass in the new size. Then read the "table" array from the file by reading "tableSize" * sizeof(DirectoryEntry) bytes from the file. Finally, move the seekposition back to the start of the directory file by calling Seek(0) on the OpenFile object.


----My implementation of synchronized file system is as follows----












------------------------TESTING-----------------------------------------


############
share1.c
sharekid1.c
############

share1.c tests that child processes inherit the same open files and file descriptors as the parent process. share1.c begins by reading in 20 characters from the file "data" (which contains the letters a-z) and then forks/execs sharekid1 which simply reads one more character, prints the character, and exits. The parent process waits for the child to exit before reading one more character.

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

share2.c tests that concurrent processes that open the same file recieve seperate, private seek positions within the file. share2.c begins by reading and printing 20 characters from the file "data" and then forks/execs sharekid2.c which opens the file "data" and reads and prints 10 characters (which should be the first 10 characters in the file) before exiting. share2.c waits for the child to exit and reads and prints one more character (which should be the 21 character from the file) before exiting.

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
