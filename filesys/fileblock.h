

#ifdef CHANGED

#ifndef FILE_BLOCK_H
#define FILE_BLOCK_H

#include "disk.h"
#include "bitmap.h"

#define MAX_BLOCKS (int) (SectorSize  / sizeof(int))
#define EMPTY_BLOCK -1

// class Block {
// public:


// };

// class DirectBlock : public Block {
// public:
// 	DirectBlock(int sector) {
// 		this->sector = sector;
// 	}

// 	int sector
// };

// class IndirectBlock : Block {
// public:


// private:
// 	Block dataSectors[NUM_SECTORS];
// };


class IndirectBlock {
public:
	IndirectBlock();

	int Allocate(BitMap *bitMap, int numSectors);// Initialize a indirect block

    void Deallocate(BitMap *bitMap);  		// De-allocate this file's 
						//  data blocks

    void FetchFrom(int sectorNumber); 	// Initialize file header from disk
    void WriteBack(int sectorNumber); 	// Write modifications to file header
					//  back to disk

    int ByteToSector(int offset);	// Convert a byte offset into the file
					// to the disk sector containing
					// the byte

    // int FileLength();			// Return the length of the file 
					// in bytes

    void Print();			// Print the contents of the file.

private:
	int dataSectors[MAX_BLOCKS];
};



class DoublyIndirectBlock {
public:
	DoublyIndirectBlock();

	int Allocate(BitMap *bitMap, int numSectors);// Initialize a indirect block

    void Deallocate(BitMap *bitMap);  		// De-allocate this file's 
						//  data blocks

    void FetchFrom(int sectorNumber); 	// Initialize file header from disk
    void WriteBack(int sectorNumber); 	// Write modifications to file header
					//  back to disk

    int ByteToSector(int offset);	// Convert a byte offset into the file
					// to the disk sector containing
					// the byte

    // int FileLength();			// Return the length of the file 
					// in bytes

    void Print();			// Print the contents of the file.

private:
	int dataSectors[MAX_BLOCKS];
};




#endif	// FILE_BLOCK_H

#endif //CHANGED