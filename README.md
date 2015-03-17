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

