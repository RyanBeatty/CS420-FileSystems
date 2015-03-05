#ifdef CHANGED


#include "fileblock.h"
#include "filehdr.h"
#include "system.h"
#include <string>

// IndirectBlock::IndirectBlock() {
// 	for(int i = 0; i < MAX_BLOCKS; ++i)
// 		dataSectors[i] = EMPTY_BLOCK;
// }

// bool 
// IndirectBlock::Allocate(BitMap *freeMap, int fileSize) { // Initialize a file header, 
// 	DEBUG('a', "starting single indirect allocation\n");
// 	ASSERT(fileSize >= 0);

// 	int numSectors = divRoundUp(fileSize, SectorSize);
// 	if(freeMap->NumClear() < numSectors)					// failure if not enough free sectors on disk
// 		return false;
// 	if(numSectors > MAX_BLOCKS)								// failure if requested number of sectors is greater than max number
// 		return false;

// 	DEBUG('a', "enough space for single indirect allocation\n");
// 	for(int i = 0; i < MAX_BLOCKS; ++i)
// 		dataSectors[i] = EMPTY_BLOCK;
// 	for(int i = 0; i < numSectors; ++i)						// allocate space for all blocks
// 		dataSectors[i] = freeMap->Find();
// 	DEBUG('a', "single indirect allocated\n");
// 	return true;
// }

int 
IndirectBlock::Allocate(BitMap *freeMap, int numSectors) { // Initialize a file header, 
	DEBUG('a', "starting single indirect allocation\n");
	if(numSectors < 0)
		return -1;

	if(freeMap->NumClear() < numSectors)					// failure if not enough free sectors on disk
		return -1;

	DEBUG('a', "enough space for single indirect allocation\n");
	int i = 0;
	for(; i < MAX_BLOCKS; ++i)
		dataSectors[i] = EMPTY_BLOCK;
	for(i = 0; i < MAX_BLOCKS && i < numSectors; ++i) {		// allocate space for all blocks
		dataSectors[i] = freeMap->Find();
		ASSERT(dataSectors[i] != EMPTY_BLOCK);
	}
	DEBUG('a', "single indirect allocated\n");
	return i;
}

void 
IndirectBlock::Deallocate(BitMap *freeMap) {
	for(int i = 0, sector; i < MAX_BLOCKS; ++i) {		// deallocate all sectors
		sector = dataSectors[i];
		ASSERT(freeMap->Test(sector));						// assert that sector to be cleared is in use
		freeMap->Clear(sector);
	}
}

void 
IndirectBlock::WriteBack(int sector) {
	synchDisk->WriteSector(sector, (char *)this);
}

void
IndirectBlock::FetchFrom(int sector) {
	synchDisk->ReadSector(sector, (char *)this);
}

int 
IndirectBlock::ByteToSector(int offset) {
	int vBlock = offset / SectorSize;
	ASSERT(vBlock < MAX_BLOCKS);				// assert that it is a valid virtual block
	int pBlock = dataSectors[vBlock];
	ASSERT(pBlock >= 0 && pBlock < NumSectors);
	return pBlock;
}


//############################################################################################################//

// DoublyIndirectBlock::DoublyIndirectBlock() {
// 	for(int i = 0; i < MAX_BLOCKS; ++i)
// 		dataSectors[i] = EMPTY_BLOCK;
// }

// bool 
// DoublyIndirectBlock::Allocate(BitMap *freeMap, int fileSize) { // Initialize a file header, 
// 	IndirectBlock *iblock;
	
// 	DEBUG('a', "starting doublyindirect allocation\n");
// 	printf("fileSize: %d\n", fileSize);
// 	ASSERT(fileSize >= 0);

// 	int numSectors = divRoundUp(fileSize, SectorSize);
// 	if(freeMap->NumClear() < numSectors)					// failure if not enough free sectors on disk
// 		return false;

// 	DEBUG('a', "enough space for doublyindirect allocation\n");
// 	for(int i = 0; i < MAX_BLOCKS; ++i)
// 		dataSectors[i] = EMPTY_BLOCK;
// 	// int allocated = 0;
// 	// for(int i = 0; i < MAX_BLOCKS; ++i, allocated += MAX_BLOCKS)	{	// allocate space for all indirect blocks
// 	// 	if(numSectors - allocated < 0)																	// stop if we have allocated enough blocks
// 	// 		break;
// 	// 	dataSectors[i] = freeMap->Find();																// allocate block for indirect block
// 	// 	iblock = new(std::nothrow) IndirectBlock;
		
// 	// 	if(numSectors - allocated > MAX_BLOCKS) {
// 	// 		ASSERT(iblock->Allocate(freeMap, MAX_BLOCKS));
// 	// 	}
// 	// 	else {
// 	// 		ASSERT(iblock->Allocate(freeMap, fileSize - (allocated * SectorSize)));							// allocate indirect block, should pass
// 	// 	}
// 	// 	iblock->WriteBack(dataSectors[i]);																// write indirect block hdr back to disk
// 	// 	delete iblock;
// 	// }
// 	for(int i = 0; i < MAX_BLOCKS; ++i, fileSize -= MAX_BLOCKS * SectorSize)	{	// allocate space for all indirect blocks
// 		if(fileSize < 0)																	// stop if we have allocated enough blocks
// 			break;
// 		dataSectors[i] = freeMap->Find();																// allocate block for indirect block
// 		iblock = new(std::nothrow) IndirectBlock;
// 		if(fileSize > MAX_BLOCKS * SectorSize) {
// 			ASSERT(iblock->Allocate(freeMap, MAX_BLOCKS * SectorSize));
// 		}
// 		else {
// 			ASSERT(iblock->Allocate(freeMap, fileSize));													// allocate indirect block, should pass
// 		}
// 		iblock->WriteBack(dataSectors[i]);																// write indirect block hdr back to disk
// 		delete iblock;
// 	}

// 	ASSERT(fileSize < 0);
// 	DEBUG('a', "doubly indirect block allocated\n");
// 	return true;
// }

int 
DoublyIndirectBlock::Allocate(BitMap *freeMap, int numSectors) { // Initialize a file header, 
	IndirectBlock *iblock;
	
	DEBUG('a', "starting doublyindirect allocation\n");
	printf("numSectors requested allocation: %d\n", numSectors);
	ASSERT(numSectors >= 0);

	if(freeMap->NumClear() < numSectors)					// failure if not enough free sectors on disk
		return -1;

	DEBUG('a', "enough space for doublyindirect allocation\n");
	int i = 0;
	for(; i < MAX_BLOCKS; ++i)
		dataSectors[i] = EMPTY_BLOCK;
	for(i = 0; i < MAX_BLOCKS && numSectors >= 0; ++i)	{	// allocate space for all indirect blocks
		dataSectors[i] = freeMap->Find();																// allocate block for indirect block
		ASSERT(dataSectors[i] != EMPTY_BLOCK);
		iblock = new(std::nothrow) IndirectBlock;
		int allocated = iblock->Allocate(freeMap, numSectors);
		ASSERT(allocated != -1);
		iblock->WriteBack(dataSectors[i]);															// write indirect block hdr back to disk
		numSectors -= allocated;
		delete iblock;
	}

	DEBUG('a', "doubly indirect block allocated\n");
	return i * MAX_BLOCKS;
}

void 
DoublyIndirectBlock::Deallocate(BitMap *freeMap) {
	IndirectBlock *iblock;
	for(int i = 0, sector; i < MAX_BLOCKS; ++i) {		// deallocate all blocks
		sector = dataSectors[i];
		if(sector == EMPTY_BLOCK)						// skip empty block
			continue;
		ASSERT(freeMap->Test(sector));					// assert that the sector we are deallocating is in use
		iblock = new(std::nothrow) IndirectBlock;
		iblock->FetchFrom(sector);						// load up filehdr
		iblock->Deallocate(freeMap);						// deallocate filehdr
		freeMap->Clear(sector);							
		delete iblock;
	}
}

void 
DoublyIndirectBlock::WriteBack(int sector) {
	synchDisk->WriteSector(sector, (char *)this);
}

void
DoublyIndirectBlock::FetchFrom(int sector) {
	synchDisk->ReadSector(sector, (char *)this);
}

int 
DoublyIndirectBlock::ByteToSector(int offset) {
	int vBlock = offset / SectorSize;											// calc virtual block we want
	IndirectBlock *iblock = new(std::nothrow) IndirectBlock;
	iblock->FetchFrom(dataSectors[vBlock / MAX_BLOCKS]);							// load up indirect block hdr that contains the virtual block we want
	int pBlock = iblock->ByteToSector((vBlock % MAX_BLOCKS) * SectorSize);		// find the corresponding physical block
	delete iblock;
	// printf("doublyindirect ByteToSector: %d\n", pBlock);
	ASSERT(pBlock >= 0 && pBlock < NumSectors);
	return pBlock;
}


#endif