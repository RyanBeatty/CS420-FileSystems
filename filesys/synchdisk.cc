#ifdef CHANGED
// synchdisk.cc 
//	Routines to synchronously access the disk.  The physical disk 
//	is an asynchronous device (disk requests return immediately, and
//	an interrupt happens later on).  This is a layer on top of
//	the disk providing a synchronous interface (requests wait until
//	the request completes).
//
//	Use a semaphore to synchronize the interrupt handlers with the
//	pending requests.  And, because the physical disk can only
//	handle one operation at a time, use a lock to enforce mutual
//	exclusion.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synchdisk.h"
#include <new>

void SectorCopy(char *dst, char *src) {
    for(int i = 0; i < SectorSize; ++i)
        dst[i] = src[i];
}

//----------------------------------------------------------------------
// DiskRequestDone
// 	Disk interrupt handler.  Need this to be a C routine, because 
//	C++ can't handle pointers to member functions.
//----------------------------------------------------------------------

static void
DiskRequestDone (int arg)
{
    SynchDisk* disk = (SynchDisk *)arg;

    disk->RequestDone();
}

//----------------------------------------------------------------------
// SynchDisk::SynchDisk
// 	Initialize the synchronous interface to the physical disk, in turn
//	initializing the physical disk.
//
//	"name" -- UNIX file name to be used as storage for the disk data
//	   (usually, "DISK")
//----------------------------------------------------------------------

SynchDisk::SynchDisk(char* name)
{
    semaphore = new(std::nothrow) Semaphore("synch disk", 0);
    lock = new(std::nothrow) Lock("synch disk lock");
    cacheLock = new(std::nothrow) Lock("cacheLock");
    disk = new(std::nothrow) Disk(name, DiskRequestDone, (int) this);
}

//----------------------------------------------------------------------
// SynchDisk::~SynchDisk
// 	De-allocate data structures needed for the synchronous disk
//	abstraction.
//----------------------------------------------------------------------

SynchDisk::~SynchDisk()
{
    delete disk;
    delete lock;
    delete semaphore;
}

//----------------------------------------------------------------------
// SynchDisk::ReadSector
// 	Read the contents of a disk sector into a buffer.  Return only
//	after the data has been read.
//
//	"sectorNumber" -- the disk sector to read
//	"data" -- the buffer to hold the contents of the disk sector
//----------------------------------------------------------------------

void
SynchDisk::ReadSector(int sectorNumber, char* data)
{
    lock->Acquire();			// only one disk I/O at a time
    cacheLock->Acquire();
    if(cache.inCache(sectorNumber)) {
        SectorCopy(data, cache.Get(sectorNumber));
        cacheLock->Release();
        lock->Release();
        return ;
    }
    cacheLock->Release();

    disk->ReadRequest(sectorNumber, data);
    semaphore->P();			// wait for interrupt
    cacheLock->Acquire();
    cache.Add(data, sectorNumber);
    cacheLock->Release();

    lock->Release();
}

//----------------------------------------------------------------------
// SynchDisk::WriteSector
// 	Write the contents of a buffer into a disk sector.  Return only
//	after the data has been written.
//
//	"sectorNumber" -- the disk sector to be written
//	"data" -- the new contents of the disk sector
//----------------------------------------------------------------------

void
SynchDisk::WriteSector(int sectorNumber, char* data)
{

    lock->Acquire();			// only one disk I/O at a time
    // cacheLock->Acquire();
    // cache.Delete(sectorNumber);
    // cacheLock->Release();
    disk->WriteRequest(sectorNumber, data);
    semaphore->P();			// wait for interrupt

    cacheLock->Acquire();
    cache.Delete(sectorNumber);
    cache.Add(data, sectorNumber);
    cacheLock->Release();

    lock->Release();
}

//----------------------------------------------------------------------
// SynchDisk::RequestDone
// 	Disk interrupt handler.  Wake up any thread waiting for the disk
//	request to finish.
//----------------------------------------------------------------------

void
SynchDisk::RequestDone()
{ 
    semaphore->V();
}
















#else

// synchdisk.cc 
//  Routines to synchronously access the disk.  The physical disk 
//  is an asynchronous device (disk requests return immediately, and
//  an interrupt happens later on).  This is a layer on top of
//  the disk providing a synchronous interface (requests wait until
//  the request completes).
//
//  Use a semaphore to synchronize the interrupt handlers with the
//  pending requests.  And, because the physical disk can only
//  handle one operation at a time, use a lock to enforce mutual
//  exclusion.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synchdisk.h"
#include <new>

//----------------------------------------------------------------------
// DiskRequestDone
//  Disk interrupt handler.  Need this to be a C routine, because 
//  C++ can't handle pointers to member functions.
//----------------------------------------------------------------------

static void
DiskRequestDone (int arg)
{
    SynchDisk* disk = (SynchDisk *)arg;

    disk->RequestDone();
}

//----------------------------------------------------------------------
// SynchDisk::SynchDisk
//  Initialize the synchronous interface to the physical disk, in turn
//  initializing the physical disk.
//
//  "name" -- UNIX file name to be used as storage for the disk data
//     (usually, "DISK")
//----------------------------------------------------------------------

SynchDisk::SynchDisk(char* name)
{
    semaphore = new(std::nothrow) Semaphore("synch disk", 0);
    lock = new(std::nothrow) Lock("synch disk lock");
    disk = new(std::nothrow) Disk(name, DiskRequestDone, (int) this);
}

//----------------------------------------------------------------------
// SynchDisk::~SynchDisk
//  De-allocate data structures needed for the synchronous disk
//  abstraction.
//----------------------------------------------------------------------

SynchDisk::~SynchDisk()
{
    delete disk;
    delete lock;
    delete semaphore;
}

//----------------------------------------------------------------------
// SynchDisk::ReadSector
//  Read the contents of a disk sector into a buffer.  Return only
//  after the data has been read.
//
//  "sectorNumber" -- the disk sector to read
//  "data" -- the buffer to hold the contents of the disk sector
//----------------------------------------------------------------------

void
SynchDisk::ReadSector(int sectorNumber, char* data)
{
    lock->Acquire();            // only one disk I/O at a time
    disk->ReadRequest(sectorNumber, data);
    semaphore->P();         // wait for interrupt
    lock->Release();
}

//----------------------------------------------------------------------
// SynchDisk::WriteSector
//  Write the contents of a buffer into a disk sector.  Return only
//  after the data has been written.
//
//  "sectorNumber" -- the disk sector to be written
//  "data" -- the new contents of the disk sector
//----------------------------------------------------------------------

void
SynchDisk::WriteSector(int sectorNumber, char* data)
{
    lock->Acquire();            // only one disk I/O at a time
    disk->WriteRequest(sectorNumber, data);
    semaphore->P();         // wait for interrupt
    lock->Release();
}

//----------------------------------------------------------------------
// SynchDisk::RequestDone
//  Disk interrupt handler.  Wake up any thread waiting for the disk
//  request to finish.
//----------------------------------------------------------------------

void
SynchDisk::RequestDone()
{ 
    semaphore->V();
}
#endif