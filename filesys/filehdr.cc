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
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return false if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

// bool
// FileHeader::Allocate(BitMap *freeMap, int fileSize)
// { 
//     DEBUG('a', "starting file header allocation\n");
//     // // ASSERT(fileSize <= MaxFileSize);
//     // numBytes = fileSize;
//     // numSectors  = divRoundUp(fileSize, SectorSize);
//     // if(numSectors > NumDirect-1 && freeMap->NumClear() < numSectors + 1)          // if we need indirect blocks, must check for extra block allocation space
//     //     return false;

//     // if (freeMap->NumClear() < numSectors)
// 	   // return false;		// not enough space

//     // printf("allocating\n");
//     // printf("numSectors: %d\n", numSectors);
//     // for (int i = 0; i < NumDirect && i < numSectors; i++) {                     // allocate blocks in bitmap
//     //     int s = freeMap->Find();
//     //     // printf("s: %d\n", s);
//     //     dataSectors[i] = s;
//     // }

//     // if(numSectors > NumDirect - 1) {                                            // handle indirection
//     //     DEBUG('f', "Allocating indirect block.\n");

//     //     printf("numSectors: %d\n", numSectors);
//     //     printf("numBytes: %d\n", numBytes);
//     //     printf("cur allocated sectors: %d\n", NumDirect - 1);
//     //     printf("cur bytes allocated: %d\n", (NumDirect - 1) * SectorSize);
//     //     printf("remaining sectors: %d\n", numSectors - (NumDirect - 1));
//     //     printf("remaining bytes: %d\n", numBytes - ((NumDirect - 1) * SectorSize));
        
//     //     int remaining = numBytes - (SectorSize * (NumDirect - 1));             // remaining bytes to be allocated
//     //     int iblockSector = dataSectors[NumDirect-1];                           // sector where iblock resides
//     //     FileHeader *iblock = new(std::nothrow) FileHeader;
//     //     ASSERT(iblock->Allocate(freeMap, remaining));                          // allocate the rest of the blocks
//     //     iblock->WriteBack(iblockSector);                                       // write back to reflect iblock allocation
//     //     delete iblock;
//     // }
    
//     // printf("allocated\n");
//     // return true;

//     numBytes = fileSize;
//     numSectors  = divRoundUp(fileSize, SectorSize);
//     if(freeMap->NumClear() < numSectors)
//        return false;     // not enough space

//     DEBUG('a', "enough space for file header\n");
//     DoublyIndirectBlock *dblock;
//     int allocated = 0;
//     for(int i = 0; i < NumDirect; ++i, allocated += MAX_BLOCKS * MAX_BLOCKS) {
//         if(numSectors - allocated < 0)
//             break;
//         dataSectors[i] = freeMap->Find();
//         dblock = new(std::nothrow) DoublyIndirectBlock;
//         ASSERT(dblock->Allocate(freeMap, fileSize - (allocated * SectorSize)));
//         dblock->WriteBack(dataSectors[i]);
//         delete dblock;
//     }

//     ASSERT(numSectors - allocated < 0);
//     DEBUG('a', "file header allocated\n");
//     return true;
// }


bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{ 
    DEBUG('a', "starting file header allocation\n");
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    if(freeMap->NumClear() < numSectors)
       return false;     // not enough space

    DEBUG('a', "enough space for file header\n");
    DoublyIndirectBlock *dblock;
    int remaining = numSectors;
    for(int i = 0; i < NumDirect && remaining >= 0; ++i) {
        dataSectors[i] = freeMap->Find();
        ASSERT(dataSectors[i] != EMPTY_BLOCK);
        dblock = new(std::nothrow) DoublyIndirectBlock;
        int allocated = dblock->Allocate(freeMap, numSectors);
        ASSERT(allocated != -1);
        dblock->WriteBack(dataSectors[i]);
        remaining -= allocated;
        delete dblock;
    }

    ASSERT(remaining <= 0);
    DEBUG('a', "file header allocated\n");
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
    for (int i = 0; i < numSectors; i++) {
	ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
	freeMap->Clear((int) dataSectors[i]);
    }
    // for(int i = 0; i < NumDirect; ++i) {
    //     ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
    //     freeMap->Clear((int) dataSectors[i]);
    // }

    // FileHeader *iblock = new(std::nothrow) FileHeader;
    // iblock->FetchFrom(dataSectors[NumDirect-1]);
    // iblock->Deallocate(freeMap);
    // delete iblock;
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
    // ASSERT(offset < numBytes);
    // int vBlock = offset / SectorSize;                                                  // calculate logical block number
    // if(vBlock >= NumDirect - 1) {                                                      // vBlock is in indirect block
    //     FileHeader *iblock = new(std::nothrow) FileHeader;
    //     iblock->FetchFrom(dataSectors[NumDirect-1]);                                    // fetch indirect block from memory
    //     int pBlock = iblock->ByteToSector((vBlock - (NumDirect - 1)) * SectorSize);   // find the corresponding physical block
    //     delete iblock;

    //     // printf("psector from iblock: %d\n", pBlock);
    //     return pBlock;
    // }

    // // printf("psector: %d\n", dataSectors[vBlock]);
    // return dataSectors[vBlock];

    int vBlock = offset / SectorSize;
    DoublyIndirectBlock *dblock = new(std::nothrow) DoublyIndirectBlock;
    dblock->FetchFrom(dataSectors[vBlock / (MAX_BLOCKS * MAX_BLOCKS)]);
    int pBlock = dblock->ByteToSector(offset);
    // printf("filehdr ByteToSector: %d\n", pBlock);
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
