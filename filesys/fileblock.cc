#ifdef CHANGED


#include "fileblock.h"
#include "filehdr.h"
#include "system.h"
#include <string>

IndirectBlock::IndirectBlock() {
	for(int i = 0; i < MAX_BLOCKS; ++i)
		dataSectors[i] = EMPTY_BLOCK;
}

int 
IndirectBlock::Allocate(BitMap *freeMap, int numSectors) { // Initialize a file header, 
	DEBUG('e', "starting single indirect allocation\n");
	if(numSectors < 0)
		return -1;

	if(freeMap->NumClear() < numSectors)					// failure if not enough free sectors on disk
		return -1;

	DEBUG('e', "enough space for single indirect allocation\n");
	int allocated = 0;
	for(int i = 0; i < MAX_BLOCKS && allocated < numSectors; ++i) {		// allocate space for all blocks
		if(dataSectors[i] != EMPTY_BLOCK)
			continue;
		dataSectors[i] = freeMap->Find();
		ASSERT(dataSectors[i] != EMPTY_BLOCK);
		++allocated;
	}

	DEBUG('e', "single indirect allocated\n");
	return allocated;
}

void 
IndirectBlock::Deallocate(BitMap *freeMap) {
	DEBUG('r', "beginning indirect block deallocation\n");
	for(int i = 0, sector; i < MAX_BLOCKS; ++i) {		// deallocate all sectors
		sector = dataSectors[i];
		if(sector == EMPTY_BLOCK)
			continue;
		ASSERT(freeMap->Test(sector));						// assert that sector to be cleared is in use
		freeMap->Clear(sector);
	}
	DEBUG('r', "finished indirect block deallocation\n");
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
DoublyIndirectBlock::DoublyIndirectBlock() {
	for(int i = 0; i < MAX_BLOCKS; ++i)
		dataSectors[i] = EMPTY_BLOCK;
}

int 
DoublyIndirectBlock::Allocate(BitMap *freeMap, int numSectors) { // Initialize a file header, 
	IndirectBlock *iblock;
	
	DEBUG('e', "starting doublyindirect allocation\n");
	// printf("numSectors requested dblock allocation: %d\n", numSectors);
	if(numSectors < 0)
		return -1;
	if(freeMap->NumClear() < numSectors)					// failure if not enough free sectors on disk
		return -1;

	DEBUG('e', "enough space for doublyindirect allocation\n");
	int allocated = 0;
	for(int i = 0; i < MAX_BLOCKS && allocated < numSectors; ++i)	{	// allocate space for all indirect blocks
		iblock = new(std::nothrow) IndirectBlock();
		if(dataSectors[i] == EMPTY_BLOCK)
			dataSectors[i] = freeMap->Find();							// allocate block for indirect block
		else
			iblock->FetchFrom(dataSectors[i]);
		ASSERT(dataSectors[i] != EMPTY_BLOCK);
		int result = iblock->Allocate(freeMap, numSectors - allocated);
		ASSERT(result >= 0);
		iblock->WriteBack(dataSectors[i]);							// write indirect block hdr back to disk
		allocated += result;
		delete iblock;
	}

	DEBUG('e', "doubly indirect block allocated\n");
	return allocated;
}

void 
DoublyIndirectBlock::Deallocate(BitMap *freeMap) {
	DEBUG('r', "beginning doublyindirect deallocation\n");
	IndirectBlock *iblock;
	for(int i = 0, sector; i < MAX_BLOCKS; ++i) {		// deallocate all blocks
		sector = dataSectors[i];
		if(sector == EMPTY_BLOCK)						// skip empty block
			continue;
		ASSERT(freeMap->Test(sector));					// assert that the sector we are deallocating is in use
		iblock = new(std::nothrow) IndirectBlock();
		iblock->FetchFrom(sector);						// load up filehdr
		iblock->Deallocate(freeMap);						// deallocate filehdr
		ASSERT(freeMap->Test(sector));					// just to be sure nothing weird happened
		freeMap->Clear(sector);							
		delete iblock;
	}
	DEBUG('r', "finished doubly indirect deallocation\n");
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
	IndirectBlock *iblock = new(std::nothrow) IndirectBlock();
	iblock->FetchFrom(dataSectors[vBlock / MAX_BLOCKS]);							// load up indirect block hdr that contains the virtual block we want
	int pBlock = iblock->ByteToSector((vBlock % MAX_BLOCKS) * SectorSize);		// find the corresponding physical block
	delete iblock;
	// printf("doublyindirect ByteToSector: %d\n", pBlock);
	ASSERT(pBlock >= 0 && pBlock < NumSectors);
	return pBlock;
}


#endif
