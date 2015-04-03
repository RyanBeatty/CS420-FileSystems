#ifdef CHANGED
// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "filehdr.h"
#include "fileblock.h"
#include <new>


//----------------------------------------------------------------------
// FileHead::FileHeader
// initialize FileHeader object. represent FileHeader for a file that
// is size 0
//----------------------------------------------------------------------

FileHeader::FileHeader() {
    for(int i = 0; i < NumDirect; ++i)
        dataSectors[i] = EMPTY_BLOCK;
    numBytes = 0;
    numSectors = 0;
}

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return false if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------


bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{ 
    DEBUG('e', "starting file header allocation\n");
    // printf("number of sectors requested for allocation: %d\n", divRoundUp(fileSize, SectorSize));
    int numReqSectors = divRoundUp(fileSize, SectorSize);
    if(freeMap->NumClear() < numReqSectors)
        return false;     // not enough space

    DEBUG('e', "enough space for file header\n");

    DoublyIndirectBlock *dblock;
    int allocated = 0;
    for(int i = 0; i < NumDirect && allocated < numReqSectors; ++i) {  // allocate all sectors
        dblock = new(std::nothrow) DoublyIndirectBlock();
        if(dataSectors[i] == EMPTY_BLOCK)
            dataSectors[i] = freeMap->Find();
        else
            dblock->FetchFrom(dataSectors[i]);
        ASSERT(dataSectors[i] != EMPTY_BLOCK);                      // assert that allocation was good
        int result = dblock->Allocate(freeMap, numReqSectors - allocated);      // allocate doubly indirect block
        ASSERT(result != -1);                                    // assert doubly indirect block allocation succeeded
        dblock->WriteBack(dataSectors[i]);                          // write doubly indirect block back
        allocated += result;                                     // decrease remaining sectors to be allocated
        delete dblock;
    }

    ASSERT(numReqSectors - allocated <= 0);
    numBytes += fileSize;
    numSectors  += divRoundUp(fileSize, SectorSize);
    DEBUG('e', "file header allocated\n");
    return true;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(BitMap *freeMap)
{
    DEBUG('r', "beginning filehdr deallocation\n");
    DoublyIndirectBlock *dblock;
    for(int i = 0, sector; i < NumDirect; ++i) {
        sector = dataSectors[i];
        if(sector == EMPTY_BLOCK)
            continue;
        ASSERT(freeMap->Test(sector));
        dblock = new(std::nothrow) DoublyIndirectBlock();
        dblock->FetchFrom(sector);
        dblock->Deallocate(freeMap);
        ASSERT(freeMap->Test(sector));
        freeMap->Clear(sector);
        delete dblock;
    }
    DEBUG('r', "finished filehdr deallocation\n");
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    synchDisk->WriteSector(sector, (char *)this); 
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
    int vBlock = offset / SectorSize;
    DoublyIndirectBlock *dblock = new(std::nothrow) DoublyIndirectBlock();
    dblock->FetchFrom(dataSectors[vBlock / (MAX_BLOCKS * MAX_BLOCKS)]);
    int pBlock = dblock->ByteToSector(offset);
    ASSERT(pBlock >= 0 && pBlock < NumSectors);
    delete dblock;
    return pBlock;
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
    char *data = new(std::nothrow) char[SectorSize];

    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    for (i = 0; i < numSectors; i++)
        printf("%d ", dataSectors[i]);
    printf("\nFile contents:\n");
    for (i = k = 0; i < numSectors; i++) {
        synchDisk->ReadSector(dataSectors[i], data);
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
            if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
                printf("%c", data[j]);
            else
                printf("\\%x", (unsigned char)data[j]);
        }
        printf("\n"); 
    }
    delete [] data;
}

#else















// filehdr.cc 
//  Routines for managing the disk file header (in UNIX, this
//  would be called the i-node).
//
//  The file header is used to locate where on disk the 
//  file's data is stored.  We implement this as a fixed size
//  table of pointers -- each entry in the table points to the 
//  disk sector containing that portion of the file data
//  (in other words, there are no indirect or doubly indirect 
//  blocks). The table size is chosen so that the file header
//  will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//  ownership, last modification date, etc., in the file header. 
//
//  A file header can be initialized in two ways:
//     for a new file, by modifying the in-memory data structure
//       to point to the newly allocated data blocks
//     for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "filehdr.h"
#include <new>

//----------------------------------------------------------------------
// FileHeader::Allocate
//  Initialize a fresh file header for a newly created file.
//  Allocate data blocks for the file out of the map of free disk blocks.
//  Return false if there are not enough free blocks to accomodate
//  the new file.
//
//  "freeMap" is the bit map of free disk sectors
//  "fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{ 
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    if (freeMap->NumClear() < numSectors)
    return false;       // not enough space

    for (int i = 0; i < numSectors; i++)
    dataSectors[i] = freeMap->Find();
    return true;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
//  De-allocate all the space allocated for data blocks for this file.
//
//  "freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(BitMap *freeMap)
{
    for (int i = 0; i < numSectors; i++) {
    ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
    freeMap->Clear((int) dataSectors[i]);
    }
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
//  Fetch contents of file header from disk. 
//
//  "sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
//  Write the modified contents of the file header back to disk. 
//
//  "sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    synchDisk->WriteSector(sector, (char *)this); 
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
//  Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//  offset in the file) to a physical address (the sector where the
//  data at the offset is stored).
//
//  "offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
    return(dataSectors[offset / SectorSize]);
}

//----------------------------------------------------------------------
// FileHeader::FileLength
//  Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
//  Print the contents of the file header, and the contents of all
//  the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
    char *data = new(std::nothrow) char[SectorSize];

    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    for (i = 0; i < numSectors; i++)
    printf("%d ", dataSectors[i]);
    printf("\nFile contents:\n");
    for (i = k = 0; i < numSectors; i++) {
    synchDisk->ReadSector(dataSectors[i], data);
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
        if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
        printf("%c", data[j]);
            else
        printf("\\%x", (unsigned char)data[j]);
    }
        printf("\n"); 
    }
    delete [] data;
}


#endif
