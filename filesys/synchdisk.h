#ifdef CHANGED
// synchdisk.h 
// 	Data structures to export a synchronous interface to the raw 
//	disk device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#ifndef SYNCHDISK_H
#define SYNCHDISK_H

#include "disk.h"
#include "synch.h"

#define CACHE_SIZE 10

void SectorCopy(char *dst, char *src);


class CacheEntry {
public:
    bool valid;
    int sector;
    char block[SectorSize];

    CacheEntry() {
        valid = false;
        sector = -1;
        bzero(block, SectorSize);
    }

    void toString() {
        printf("Entry\n");
        printf("\tvalid: %s\n", valid ? "true" : "false");
        printf("\tsector: %d\n", sector);
    }
};

class Cache {
public:
    CacheEntry entries[CACHE_SIZE];

    Cache() {
        for(int i = 0; i < CACHE_SIZE; ++i)
            entries[i].valid = false;
    }

    bool inCache(int sector) {
        return IndexOf(sector) != -1? true : false;
    }

    char *Get(int sector) {
        int i = IndexOf(sector);
        return i != -1 ? entries[i].block : NULL;
    }

    void Add(char *data, int sector) {
        for(int i = 0; i < CACHE_SIZE; ++i) {
            if(!entries[i].valid) {
                entries[i].valid = true;
                entries[i].sector = sector;
                SectorCopy(entries[i].block, data);
                return ;
            }
        }

        int victim = Random() % CACHE_SIZE;
        entries[victim].sector = sector;
        SectorCopy(entries[victim].block, data);
    }

    void Delete(int sector) {
        for(int i = 0; i < CACHE_SIZE; ++i) {
            if(entries[i].sector == sector) {
                entries[i].valid = false;
                return ;
            }
        }
    }

    void toString() {
        printf("--------------\n");
        printf("CACHE\n");
        printf("--------------\n");
        for(int i = 0; i < CACHE_SIZE; ++i)
            entries[i].toString();
    }
private:
    int IndexOf(int sector) {
        for(int i = 0; i < CACHE_SIZE; ++i) {
            if(entries[i].valid && entries[i].sector == sector)
                return i;
        }

        return -1;
    }
};




// The following class defines a "synchronous" disk abstraction.
// As with other I/O devices, the raw physical disk is an asynchronous device --
// requests to read or write portions of the disk return immediately,
// and an interrupt occurs later to signal that the operation completed.
// (Also, the physical characteristics of the disk device assume that
// only one operation can be requested at a time).
//
// This class provides the abstraction that for any individual thread
// making a request, it waits around until the operation finishes before
// returning.
class SynchDisk {
  public:
    SynchDisk(char* name);    		// Initialize a synchronous disk,
					// by initializing the raw Disk.
    ~SynchDisk();			// De-allocate the synch disk data
    
    void ReadSector(int sectorNumber, char* data);
    					// Read/write a disk sector, returning
    					// only once the data is actually read 
					// or written.  These call
    					// Disk::ReadRequest/WriteRequest and
					// then wait until the request is done.
    void WriteSector(int sectorNumber, char* data);
    
    void RequestDone();			// Called by the disk device interrupt
					// handler, to signal that the
					// current disk operation is complete.

  private:
    Disk *disk;		  		// Raw disk device
    Semaphore *semaphore; 		// To synchronize requesting thread 
					// with the interrupt handler
    Lock *lock;		  		// Only one read/write request
					// can be sent to the disk at a time
    Lock *cacheLock;
    Cache cache;
};

#endif // SYNCHDISK_H


















#else


// synchdisk.h 
//  Data structures to export a synchronous interface to the raw 
//  disk device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#ifndef SYNCHDISK_H
#define SYNCHDISK_H

#include "disk.h"
#include "synch.h"

// The following class defines a "synchronous" disk abstraction.
// As with other I/O devices, the raw physical disk is an asynchronous device --
// requests to read or write portions of the disk return immediately,
// and an interrupt occurs later to signal that the operation completed.
// (Also, the physical characteristics of the disk device assume that
// only one operation can be requested at a time).
//
// This class provides the abstraction that for any individual thread
// making a request, it waits around until the operation finishes before
// returning.
class SynchDisk {
  public:
    SynchDisk(char* name);          // Initialize a synchronous disk,
                    // by initializing the raw Disk.
    ~SynchDisk();           // De-allocate the synch disk data
    
    void ReadSector(int sectorNumber, char* data);
                        // Read/write a disk sector, returning
                        // only once the data is actually read 
                    // or written.  These call
                        // Disk::ReadRequest/WriteRequest and
                    // then wait until the request is done.
    void WriteSector(int sectorNumber, char* data);
    
    void RequestDone();         // Called by the disk device interrupt
                    // handler, to signal that the
                    // current disk operation is complete.

  private:
    Disk *disk;             // Raw disk device
    Semaphore *semaphore;       // To synchronize requesting thread 
                    // with the interrupt handler
    Lock *lock;             // Only one read/write request
                    // can be sent to the disk at a time
};

#endif // SYNCHDISK_H

#endif