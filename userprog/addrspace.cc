#ifdef CHANGED

// addrspace.cc 
//  Routines to manage address spaces (executing user programs).
//
//  In order to run a user program, you must:
//
//  1. link with the -N -T 0 option 
//  2. run coff2noff to convert the object file to Nachos format
//      (Nachos object code format is essentially just a simpler
//      version of the UNIX executable object code format)
//  3. load the NOFF file into the Nachos file system
//      (if you haven't implemented the file system yet, you
//      don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include <new>

extern BitMap *bitMap;

//----------------------------------------------------------------------
// SwapHeader
//  Do little endian to big endian conversion on the bytes in the 
//  object file header, in case the file was generated on a little
//  endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
    noffH->noffMagic = WordToHost(noffH->noffMagic);
    noffH->code.size = WordToHost(noffH->code.size);
    noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
    noffH->initData.size = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

AddrSpace::AddrSpace(OpenFile *executable, bool chckpnt) {
        NoffHeader noffH;
    unsigned int size;
#ifndef USE_TLB
    unsigned int i;
#endif
    id = processId++;

    wdSector = 1;               // point to home directory sector

    is_checkpoint = chckpnt;
    if(chckpnt) {
        rebuild_system(executable);
        return;
    }



    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
        (WordToHost(noffH.noffMagic) == NOFFMAGIC))
        SwapHeader(&noffH);
    if(noffH.noffMagic != NOFFMAGIC) {                      // if we arent running a valid binary, signal failure
        success = false;
        return ;
    }

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
            + UserStackSize;    // we need to increase the size
                        // to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    // if(!(numPages <= NumPhysPages)) {                           // check we're not trying
    //     success = false;                                        // to run anything too big --
    //     return ;                                                // at least until we have
    // }                                                           // virtual memory         
    

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
                    numPages, size);


    // ################################  NACHOS 3 CODE FOR VIRTUAL MEMORY UNDER CONSTRUCTION ###############
    // printf("num clear: %d, size: %d\n", diskMap->NumClear(), numPages);
    fflush(stdout);
    if((int) numPages > diskMap->NumClear()) {
        // printf("too big\n");
        // fflush(stdout);
        success = false;
        return ;
    }




    char *page = new(std::nothrow) char[size];                  // used to contain instructions and data for binary
    bzero(page, size);

    sectorMap = new(std::nothrow) int[numPages];                // map of virtual page numbers to disk sectors
    bzero(sectorMap, numPages);


    // copy code and data segments into disk buffer
    if (noffH.code.size > 0) {                      
        executable->ReadAt(&page[noffH.code.virtualAddr], noffH.code.size, noffH.code.inFileAddr);
    }
    if (noffH.initData.size > 0) {
        executable->ReadAt(&page[noffH.initData.virtualAddr],noffH.initData.size, noffH.initData.inFileAddr);
    }

    // printf("num free: %d\n", diskMap->NumClear());
    //     fflush(stdout);

    // iterate over each page, and write each page to disk
    for(unsigned int i = 0; i < numPages; ++i) {
        int nextFreeSector = diskMap->Find();               // find next free disk sector
        if(nextFreeSector == -1) {
            // printf("no success\n");
            success = false;
            return;
            // ASSERT(false);
        }

        sectorMap[i] = nextFreeSector;                      // save mapping of virtual page to disk sectors
        // vmDisk->WriteSector(nextFreeSector, &page[i*PageSize]);      // write virtual page i to disk
        vmFile->WriteAt(&page[i*PageSize], SectorSize, nextFreeSector * SectorSize);
    }

    delete [] page;

    //Find the next available space in physical memory based on numPages
    //Zero out the corresponding memory space for the desired page

    pageMap = new(std::nothrow) int[numPages];
    for(unsigned int i = 0; i < numPages; ++i)
        pageMap[i] = -1;


    // #####################################################################################################

    


#ifndef USE_TLB
// first, set up the translation 
    pageTable = new(std::nothrow) TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
    pageTable[i].virtualPage = i;   // for now, virtual page # = phys page #
    pageTable[i].physicalPage = i;
    pageTable[i].valid = true;
    pageTable[i].use = false;
    pageTable[i].dirty = false;
    pageTable[i].readOnly = false;  // if the code segment was entirely on 
                    // a separate page, we could set its 
                    // pages to be read-only
    }
#endif    

/*
// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
    bzero(machine->mainMemory, size);
*/

// then, copy in the code and data segments into memory
    // if (noffH.code.size > 0) {
    //     DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
    //         V2P(noffH.code.virtualAddr), noffH.code.size);
    //     executable->ReadAt(&(machine->mainMemory[V2P(noffH.code.virtualAddr)]),
    //         noffH.code.size, noffH.code.inFileAddr);
    // }
    // if (noffH.initData.size > 0) {
    //     DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
    //         V2P(noffH.initData.virtualAddr), noffH.initData.size);
    //     executable->ReadAt(&(machine->mainMemory[V2P(noffH.initData.virtualAddr)]),
    //         noffH.initData.size, noffH.initData.inFileAddr);
    // }

    // fileTable = new(std::nothrow) OpenFileTable();          // each process should have its own List of open files
    success = true;
}


void
AddrSpace::rebuild_system(OpenFile *f) {
    // printf("start rebuild\n");

    char tmp = '\0';
    f->Read(&tmp, 1);      // read newline char


    save_registers = new(std::nothrow) int[NumTotalRegs];
    bzero(save_registers, NumTotalRegs);
    char regBuffer[30];
    for(int i = 0; i < NumTotalRegs; ++i) {             // rebuild registers
        memset(regBuffer, '\0', sizeof(regBuffer));

        f->Read(&tmp, 1);
        for(int j = 0; tmp != '\n'; ++j) {      // get data for register
            regBuffer[j] = tmp;
            f->Read(&tmp, 1);
        }

        int data = atoi(regBuffer);
        save_registers[i] = data;
        // machine->WriteRegister(i, data);
        // printf("rebuild reg %d: %d\n", i, data);
    }

    // machine->WriteRegister(34, machine->ReadRegister(34) + 4);
    // machine->WriteRegister(35, machine->ReadRegister(35) + 4);
    // machine->WriteRegister(36, machine->ReadRegister(36) + 4);

    char buffer[30];
    memset(buffer, '\0', sizeof(buffer));
    tmp = '\0';
    f->Read(&tmp, 1);
    for(int i = 0; tmp != '\n'; ++i) {
        buffer[i] = tmp;
        f->Read(&tmp, 1);
    }

    numPages = atoi(buffer);
    // printf("p: %d\n", numPages);

    memLock->Acquire();
    sectorMap = new(std::nothrow) int[numPages];
    bzero(sectorMap, numPages);

    char pageBuffer[PageSize];
    for(unsigned int i = 0; i < numPages; ++i) {
        memset(pageBuffer, '\0', PageSize);

        int nextFreeSector = diskMap->Find();               // find next free disk sector
        if(nextFreeSector == -1) {
            // printf("no success\n");
            success = false;
            memLock->Release();
            return;
            // ASSERT(false);
        }

        sectorMap[i] = nextFreeSector;                      // save mapping of virtual page to disk sectors
        f->Read(pageBuffer, PageSize);
        synchDisk->WriteSector(nextFreeSector, pageBuffer);      // write virtual page i to disk
    }

    memLock->Release();

    success = true;

    pageMap = new(std::nothrow) int[numPages];
    for(unsigned int i = 0; i < numPages; ++i)
        pageMap[i] = -1;



    // printf("end rebuild\n");

    return;
}

void
AddrSpace::restore_registers() {
    for(int i = 0; i < NumTotalRegs; ++i)
        machine->WriteRegister(i, save_registers[i]);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
//  Create an address space to run a user program.
//  Load the program from a file "executable", and set everything
//  up so that we can start executing user instructions.
//
//  Assumes that the object code file is in NOFF format.
//
//  First, set up the translation from program memory to physical 
//  memory.  For now, this is really simple (1:1), since we are
//  only uniprogramming, and we have a single unsegmented page table
//
//  "executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{
    NoffHeader noffH;
    unsigned int size;
#ifndef USE_TLB
    unsigned int i;
#endif
    id = processId++;
    is_checkpoint = false;


    wdSector = 1;                                               // point to home directory sector

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
        (WordToHost(noffH.noffMagic) == NOFFMAGIC))
        SwapHeader(&noffH);
    if(noffH.noffMagic != NOFFMAGIC) {                      // if we arent running a valid binary, signal failure
        success = false;
        return ;
    }

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
            + UserStackSize;    // we need to increase the size
                        // to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    // if(!(numPages <= NumPhysPages)) {                           // check we're not trying
    //     success = false;                                        // to run anything too big --
    //     return ;                                                // at least until we have
    // }                                                           // virtual memory         
    

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
                    numPages, size);


    // ################################  NACHOS 3 CODE FOR VIRTUAL MEMORY UNDER CONSTRUCTION ###############
    // printf("num clear: %d, size: %d\n", diskMap->NumClear(), numPages);
    fflush(stdout);
    if((int) numPages > diskMap->NumClear()) {
        // printf("too big\n");
        // fflush(stdout);
        success = false;
        return ;
    }




    char *page = new(std::nothrow) char[size];                  // used to contain instructions and data for binary
    bzero(page, size);

    sectorMap = new(std::nothrow) int[numPages];                // map of virtual page numbers to disk sectors
    bzero(sectorMap, numPages);


    // copy code and data segments into disk buffer
    if (noffH.code.size > 0) {                      
        executable->ReadAt(&page[noffH.code.virtualAddr], noffH.code.size, noffH.code.inFileAddr);
    }
    if (noffH.initData.size > 0) {
        executable->ReadAt(&page[noffH.initData.virtualAddr],noffH.initData.size, noffH.initData.inFileAddr);
    }

    // printf("num free: %d\n", diskMap->NumClear());
    //     fflush(stdout);

    // iterate over each page, and write each page to disk
    for(unsigned int i = 0; i < numPages; ++i) {
        memLock->Acquire();
        int nextFreeSector = diskMap->Find();               // find next free disk sector
        if(nextFreeSector == -1) {
            // printf("no success\n");
            success = false;
            memLock->Release();
            return;
            // ASSERT(false);
        }

        sectorMap[i] = nextFreeSector;                      // save mapping of virtual page to disk sectors
        // vmDisk->WriteSector(nextFreeSector, &page[i*PageSize]);      // write virtual page i to disk
        vmFile->WriteAt(&page[i*PageSize], SectorSize, nextFreeSector * SectorSize);
        memLock->Release();
    }

    delete [] page;

    //Find the next available space in physical memory based on numPages
    //Zero out the corresponding memory space for the desired page

    pageMap = new(std::nothrow) int[numPages];
    for(unsigned int i = 0; i < numPages; ++i)
        pageMap[i] = -1;


    // #####################################################################################################

    


#ifndef USE_TLB
// first, set up the translation 
    pageTable = new(std::nothrow) TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
    pageTable[i].virtualPage = i;   // for now, virtual page # = phys page #
    pageTable[i].physicalPage = i;
    pageTable[i].valid = true;
    pageTable[i].use = false;
    pageTable[i].dirty = false;
    pageTable[i].readOnly = false;  // if the code segment was entirely on 
                    // a separate page, we could set its 
                    // pages to be read-only
    }
#endif    

/*
// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
    bzero(machine->mainMemory, size);
*/

// then, copy in the code and data segments into memory
    // if (noffH.code.size > 0) {
    //     DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
    //         V2P(noffH.code.virtualAddr), noffH.code.size);
    //     executable->ReadAt(&(machine->mainMemory[V2P(noffH.code.virtualAddr)]),
    //         noffH.code.size, noffH.code.inFileAddr);
    // }
    // if (noffH.initData.size > 0) {
    //     DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
    //         V2P(noffH.initData.virtualAddr), noffH.initData.size);
    //     executable->ReadAt(&(machine->mainMemory[V2P(noffH.initData.virtualAddr)]),
    //         noffH.initData.size, noffH.initData.inFileAddr);
    // }

    // fileTable = new(std::nothrow) OpenFileTable();          // each process should have its own List of open files
    success = true;
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//  Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
#ifndef USE_TLB
   delete pageTable;
#endif
}

//----------------------------------------------------------------------
// AddrSpace::V2P()
//  Takes in a virtual address and translates the address to its corresponding
//  physical address in memory.
//----------------------------------------------------------------------
int
AddrSpace::V2P(int virtual_addr) {
    int page_num = virtual_addr / PageSize;                  // gives us page number is VAS
    int offset = virtual_addr % PageSize;                       // 

    if(pageMap[page_num] == -1) {
        // printf("start v2p load in page\n");
        memLock->Release();
        machine->WriteRegister(39, virtual_addr);
        ExceptionHandler(PageFaultException);
        memLock->Acquire();
        // printf("end v2p load in page\n");
    }

    int frame_num = pageMap[page_num];    // gives us corresponding page_frame number
    return (frame_num * PageSize) + offset;                     // return physical address
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
//  Set the initial values for the user-level register set.
//
//  We write these directly into the "machine" registers, so
//  that we can immediately jump to user code.  Note that these
//  will be saved/restored into the currentThread->userRegisters
//  when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
    machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);   

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
//  On a context switch, save any machine state, specific
//  to this address space, that needs saving.
//
//  For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{
    for(int i = 0; i < TLBSize; ++i) {
        // if(machine->tlb[i].valid && machine->tlb[i].dirty) {
        //     int virtPage = machine->tlb[i].virtualPage;
        //     int physPage = machine->tlb[i].physicalPage;
        //     synchDisk->WriteSector(sectorMap[virtPage], &machine->mainMemory[physPage * PageSize]);
        //     machine->tlb[i].dirty = false; 
        // }

        machine->tlb[i].valid = false;
    }
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
//  On a context switch, restore the machine state so that
//  this address space can run.
//
//      For now, tell the machine where to find the page table,
//      IF address translation is done with a page table instead
//      of a hardware TLB.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
#ifndef USE_TLB
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
#endif
}

//----------------------------------------------------------------------
//  Addrspace::GetPageMap()
//   getter method for the pagemap
//----------------------------------------------------------------------
int *
AddrSpace::GetPageMap() {
    return pageMap;
}

SpaceId
AddrSpace::GetId() {
    return id;
}

bool
AddrSpace::GetSuccess() {
    return success;
}

unsigned int
AddrSpace::GetNumPages() {
    return numPages;
}


#else

// addrspace.cc 
//  Routines to manage address spaces (executing user programs).
//
//  In order to run a user program, you must:
//
//  1. link with the -N -T 0 option 
//  2. run coff2noff to convert the object file to Nachos format
//      (Nachos object code format is essentially just a simpler
//      version of the UNIX executable object code format)
//  3. load the NOFF file into the Nachos file system
//      (if you haven't implemented the file system yet, you
//      don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include <new>

//----------------------------------------------------------------------
// SwapHeader
//  Do little endian to big endian conversion on the bytes in the 
//  object file header, in case the file was generated on a little
//  endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
    noffH->noffMagic = WordToHost(noffH->noffMagic);
    noffH->code.size = WordToHost(noffH->code.size);
    noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
    noffH->initData.size = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
//  Create an address space to run a user program.
//  Load the program from a file "executable", and set everything
//  up so that we can start executing user instructions.
//
//  Assumes that the object code file is in NOFF format.
//
//  First, set up the translation from program memory to physical 
//  memory.  For now, this is really simple (1:1), since we are
//  only uniprogramming, and we have a single unsegmented page table
//
//  "executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{
    NoffHeader noffH;
    unsigned int size;
#ifndef USE_TLB
    unsigned int i;
#endif

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
        (WordToHost(noffH.noffMagic) == NOFFMAGIC))
        SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
            + UserStackSize;    // we need to increase the size
                        // to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    ASSERT(numPages <= NumPhysPages);       // check we're not trying
                        // to run anything too big --
                        // at least until we have
                        // virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
                    numPages, size);
#ifndef USE_TLB
// first, set up the translation 
    pageTable = new(std::nothrow) TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
    pageTable[i].virtualPage = i;   // for now, virtual page # = phys page #
    pageTable[i].physicalPage = i;
    pageTable[i].valid = true;
    pageTable[i].use = false;
    pageTable[i].dirty = false;
    pageTable[i].readOnly = false;  // if the code segment was entirely on 
                    // a separate page, we could set its 
                    // pages to be read-only
    }
#endif    

// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
    bzero(machine->mainMemory, size);

// then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
            noffH.code.virtualAddr, noffH.code.size);
        executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),
            noffH.code.size, noffH.code.inFileAddr);
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
            noffH.initData.virtualAddr, noffH.initData.size);
        executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]),
            noffH.initData.size, noffH.initData.inFileAddr);
    }

    fileTable = new(std::nothrow) OpenFileTable();          // each process should have its own List of open files

}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//  Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
#ifndef USE_TLB
   delete pageTable;
#endif
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
//  Set the initial values for the user-level register set.
//
//  We write these directly into the "machine" registers, so
//  that we can immediately jump to user code.  Note that these
//  will be saved/restored into the currentThread->userRegisters
//  when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
    machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);   

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
//  On a context switch, save any machine state, specific
//  to this address space, that needs saving.
//
//  For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
//  On a context switch, restore the machine state so that
//  this address space can run.
//
//      For now, tell the machine where to find the page table,
//      IF address translation is done with a page table instead
//      of a hardware TLB.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
#ifndef USE_TLB
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
#endif
}

#endif //CHANGED
