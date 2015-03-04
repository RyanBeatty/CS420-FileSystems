#ifdef CHANGED


#include "fileblock.h"
#include "filehdr.h"
#include "system.h"

IndirectBlock::IndirectBlock() {
	for(int i = 0; i < MAX_BLOCKS; ++i)
		dataSectors[i] = EMPTY_BLOCK;
}

bool 
IndirectBlock::Allocate(BitMap *freeMap, int fileSize) { // Initialize a file header, 
	if(fileSize < 0)										// signal failure if request for allocation is negative
		return false;

	int numSectors = divRoundUp(fileSize, SectorSize);
	if(freeMap->NumClear() < numSectors)					// failure if not enough free sectors on disk
		return false;
	if(numSectors > MAX_BLOCKS)								// failure if requested number of sectors is greater than max number
		return false;

	for(int i = 0; i < numSectors; ++i)						// allocate space for all blocks
		dataSectors[i] = freeMap->Find();
	return true;
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
	return dataSectors[vBlock];
}


//############################################################################################################//

DoublyIndirectBlock::DoublyIndirectBlock() {
	for(int i = 0; i < MAX_BLOCKS; ++i)
		dataSectors[i] = EMPTY_BLOCK;
}

bool 
DoublyIndirectBlock::Allocate(BitMap *freeMap, int fileSize) { // Initialize a file header, 
	FileHeader *fhdr;

	if(fileSize < 0)										// signal failure if request for allocation is negative
		return false;

	int numSectors = divRoundUp(fileSize, SectorSize);
	if(freeMap->NumClear() < numSectors + MAX_BLOCKS)		// failure if not enough free sectors on disk
		return false;

	for(int i = 0, allocated = 0; i < MAX_BLOCKS && i < numSectors; ++i, allocated += MAX_BLOCKS)	{	// allocate space for all indirect blocks
		if(numSectors - allocated > 0)																	// stop if we have allocated enough blocks
			break;
		dataSectors[i] = freeMap->Find();																// allocate block for indirect block
		fhdr = new(std::nothrow) FileHeader;
		ASSERT(fhdr->Allocate(freeMap, fileSize - (allocated * SectorSize)));							// allocate indirect block, should pass
		fhdr->WriteBack(dataSectors[i]);																// write indirect block hdr back to disk
		delete fhdr;
	}

	return true;
}

void 
DoublyIndirectBlock::Deallocate(BitMap *freeMap) {
	IndirectBlock *fhdr;
	for(int i = 0, sector; i < MAX_BLOCKS; ++i) {		// deallocate all blocks
		sector = dataSectors[i];
		if(sector == EMPTY_BLOCK)						// skip empty block
			continue;
		ASSERT(freeMap->Test(sector));					// assert that the sector we are deallocating is in use
		fhdr = new(std::nothrow) IndirectBlock();
		fhdr->FetchFrom(sector);						// load up filehdr
		fhdr->Deallocate(freeMap);						// deallocate filehdr
		freeMap->Clear(sector);							
		delete fhdr;
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
	int vBlock = offset / SectorSize;							// calc virtual block we want
	IndirectBlock *fhdr = new(std::nothrow) IndirectBlock();
	fhdr->FetchFrom(dataSectors[vBlock / MAX_BLOCKS]);			// load up indirect block hdr that contains the virtual block we want
	int pBlock = fhdr->ByteToSector(vBlock % MAX_BLOCKS);		// find the corresponding physical block
	delete fhdr;
	return pBlock;
}


#endif