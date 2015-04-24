// addrspace.h 
//  Data structures to keep track of executing user programs 
//  (address spaces).
//
//  For now, we don't keep any information about address spaces.
//  The user level CPU state is saved and restored in the thread
//  executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
#ifdef CHANGED 
// addrspace.h 
//  Data structures to keep track of executing user programs 
//  (address spaces).
//
//  For now, we don't keep any information about address spaces.
//  The user level CPU state is saved and restored in the thread
//  executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "filetable.h"

#define UserStackSize       1024    // increase this as necessary!

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable, bool is_checkpoint);
    AddrSpace(OpenFile *executable);    // Create an address space,
                    // initializing it with the program
                    // stored in the file "executable"
    ~AddrSpace();           // De-allocate an address space

    void InitRegisters();       // Initialize user-level CPU registers,
                    // before jumping to user code

    void SaveState();           // Save/restore address space-specific
    void RestoreState();        // info on a context switch

    FileVector *fileVector;         // per process file vector containging open file descriptors

    int V2P(int vAddr);        // resolves virtual address to physical address

    int *GetPageMap();
    SpaceId GetId();
    bool GetSuccess();
    unsigned int GetNumPages();

    void restore_registers();

    void rebuild_system(OpenFile *f);

    int *sectorMap;             // mapping of virtual pages to disk sectors
    int *pageMap;               //contains mapping from virtual page number to page frame. 
                                //index corresponds to page num, value corresponds to page frame num

    bool is_checkpoint;

    int *save_registers;

    int wdSector;

  private:
#ifndef USE_TLB
    TranslationEntry *pageTable;    // Assume linear page table translation
#endif                  // for now!
    unsigned int numPages;      // Number of pages in the virtual 
                               // address space
    

    SpaceId id;
    bool success;               // boolean. If true, construction of addrspace passed, else construction failed and we should not run the binary

};

#endif // ADDRSPACE_H

#else
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "filetable.h"

#define UserStackSize       1024    // increase this as necessary!

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);    // Create an address space,
                    // initializing it with the program
                    // stored in the file "executable"
    ~AddrSpace();           // De-allocate an address space

    void InitRegisters();       // Initialize user-level CPU registers,
                    // before jumping to user code

    void SaveState();           // Save/restore address space-specific
    void RestoreState();        // info on a context switch

    OpenFileTable *fileTable;

  private:
#ifndef USE_TLB
    TranslationEntry *pageTable;    // Assume linear page table translation
#endif                  // for now!
    unsigned int numPages;      // Number of pages in the virtual 
                    // address space
};

#endif // ADDRSPACE_H
#endif //CHANGED