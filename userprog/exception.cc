#ifdef CHANGED
// exception.cc 
//  Entry point into the Nachos kernel from user programs.
//  There are two kinds of things that can cause control to
//  transfer back to here from user code:
//
//  syscall -- The user code explicitly requests to call a procedure
//  in the Nachos kernel.  Right now, the only function we support is
//  "Halt".
//
//  exceptions -- The user code does something that the CPU can't handle.
//  For instance, accessing memory that doesn't exist, arithmetic errors,
//  etc.  
//
//  Interrupts (which can also cause control to transfer from user
//  code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "filetable.h"
#include "synchconsole.h"
#include "processlist.h"

#include "string.h"

#define MAX_STRING_LENGTH 128           // max length of a user-land string including null character

#define CHECKPOINT_TAG "#CHECKPOINT#"

int find_virtual_page(int physicalPage);

//----------------------------------------------------------------------
// LoadStringFromMemory()
//
//  Copies a string from user land into kernal land
//
// "pAddr" - virtual address location of first byte in string
//----------------------------------------------------------------------
char *
LoadStringFromMemory(int vAddr) {

    // printf("start load\n");
    bool terminated = false;
    char *buffer = new(std::nothrow) char[MAX_STRING_LENGTH];
    
    memLock->Acquire();
    // printf("ld str acquire\n");
    for(int i = 0; i < MAX_STRING_LENGTH; ++i) {                   // iterate until max string length
        int pAddr = currentThread->space->V2P(vAddr + i);
        if((buffer[i] = machine->mainMemory[pAddr]) == '\0') {      // break if string ended
          terminated = true;
          break;
        }
    }

    memLock->Release();
    // printf("end load\n");

    if(!terminated) {                         // if the string is longer than the max string length, return NULL
        delete [] buffer;
        return NULL;
    }

    buffer[MAX_STRING_LENGTH - 1] = '\0';                       // truncate string to protect against overflow
    return buffer;
}

//----------------------------------------------------------------------
//  SaveStringToMemory()
//   Takes a buffer and saves it back to its place in main memory
//
//  "buffer" - the buffer to save
//  "numRead" - the number of bytes read
//  "pAddr" - the base virtual address of the buffer in main memory
//----------------------------------------------------------------------
void
SaveStringToMemory(char* buffer, int numRead, int vAddr) {
    // printf("start read\n");
    memLock->Acquire();
    for(int i = 0; i < numRead; ++i) {                   // iterate over the amount of bytes read
        int pAddr = currentThread->space->V2P(vAddr + i);
        machine->mainMemory[pAddr] = buffer[i];     // copy buffer from kernel-land back to user-land
    }
    // printf("end read\n");
    memLock->Release();

    return ;
}

//----------------------------------------------------------------------
//  StartProcess()
//   called by new process threads in order to start executing user code.
//  hooks up a new addrspace object that contains data for the user binary
//  to the new process, initiliazes the machines registers, and runs the binary
//
//  "addrspace" - an addrspace object cast to an int that contains the data for the user binary
//----------------------------------------------------------------------
void StartProcess(int args) {
    char **argStrings = (char **) args;

    if(currentThread->space->is_checkpoint) {
        currentThread->space->restore_registers();
        // printf("machine run\n");
        currentThread->space->RestoreState();      // load page table register
        machine->Run();
    }
    

    currentThread->space->InitRegisters();     // set the initial register values
    currentThread->space->RestoreState();      // load page table register


    int argc = 0;                                       // calculate the number of arguemts to be passed
    int sp = machine->ReadRegister(StackReg);           // grab a copy of the stack pointer

    if(argStrings != NULL) {

        for(int i = 0; argStrings[i] != 0; ++i, ++argc);

        int argvAddr[argc];                                 // create an arrray to hold the adddresses of the arguments

        memLock->Acquire();
        // printf("startprocess acquire\n");
        for(int i = 0; argStrings[i] != 0; ++i) {               // for each argument, copy it over to the stack
            int len = strlen(argStrings[i]) + 1;                // calc str len of the argument
            sp -= len;                                          // move the stack pointer by that amount

            for(int j = 0; j < len; ++j)                                            
                machine->mainMemory[currentThread->space->V2P(sp + j)] = argStrings[i][j];      // copy the string over
            argvAddr[i] = sp;                                                                   // store the virtual address of the argument
        }


        sp = sp & ~3;                                   // calculate offset


        sp -= sizeof(int) * argc;

        for(int i = 0; i < argc; ++i)
            *(unsigned int *) &machine->mainMemory[currentThread->space->V2P(sp + i*4)] = WordToMachine((unsigned int) argvAddr[i]);
        // printf("startprocess release\n");
        memLock->Release();
    }

    machine->WriteRegister(4, argc);                // store argc for return
    machine->WriteRegister(5, sp);                  // store stack pointer for return 


    machine->WriteRegister(StackReg, sp - 8);


    machine->Run();
    ASSERT(false);                  // should never return from machine
}

//----------------------------------------------------------------------
//  SC_CREATE()
//
//  Implementation for Create() system call in exception handler.
//  Loads the filename argument string from user-land memory.
//  Then creates the file and increments the program counter to point
//  to the next instruction
//----------------------------------------------------------------------
void 
SC_CREATE() {
    char *filename = LoadStringFromMemory(machine->ReadRegister(4));     // grab filename argument from register
    if(filename == NULL)    // cant load filename string, so error
        return ;

    DEBUG('a', "filename: %s\n", filename);
    fileSystem->Create(filename, 0, currentThread->space->wdSector);                    // attempt to create a new file

    delete [] filename;
    return ;
}

//----------------------------------------------------------------------
//  SC_OPEN()
//    Open a file specified by the argument, and return an OpenFileId to the file
//    or a -1 on error. 
//----------------------------------------------------------------------
int
SC_OPEN(){
    char *filename = LoadStringFromMemory(machine->ReadRegister(4));     // grab filename argument from register    
    if(filename == NULL)   // cant load filename string, so error
        return -1;

    OpenFile *f = fileSystem->Open(filename, currentThread->space->wdSector);
    delete [] filename;
    if(f == NULL)        // cant open file, so error
        return -1;

    OpenFileId id = currentThread->space->fileVector->Insert(f);
    return id;                                                          // return OpenFileId to file
}


//----------------------------------------------------------------------
//  SC_CLOSE()
//    remove one reference to the corresponding OpenFile in the 
//  OpenFileTable
//----------------------------------------------------------------------
void
SC_CLOSE(){
    OpenFileId id = machine->ReadRegister(4);                   // grab fileid to close
    currentThread->space->fileVector->Remove(id);                // decrement a reference count to that OpenFile object in the OpenFileTable
    return ;
}

//----------------------------------------------------------------------
//  SC_WRITE()
//
//  implements the Write() system call. takes in a buffer of characters to write,
//  the amount of bytes to write, and an OpenFileId to an open file and writes
//  size bytes to the file.
//----------------------------------------------------------------------
void 
SC_WRITE() {
    // printf("start write\n");
    char *buffer = LoadStringFromMemory(machine->ReadRegister(4));     // grab buffer argument from register    
    if(buffer == NULL)     // error bad input
        return ;

    int size = machine->ReadRegister(5);                              // grab number of bytes to write
    OpenFileId id = machine->ReadRegister(6);                         // grab fileid of the file we want to write to


    if(id == ConsoleOutput) {                                       // if we want to Write to ConsoleOutput, use the SynchConsole
        
        ioLock->Acquire();
        char *curChar = buffer;                                     // iterate over the writebuffer and write out each character to ConsoleOutput
        while(size-- > 0)                         
            synchConsole->WriteChar(*curChar++);
        ioLock->Release();
    } 
    else {                                                           // else we are trying to write to an OpenFile
        
        ioLock->Acquire();
        OpenFile *f = currentThread->space->fileVector->Resolve(id);   // resolve the fileid to an OpenFile Object using the OpenFileTable
        if(f == NULL) {     // trying to read from bad fileid
            delete [] buffer;
            ioLock->Release();
            return ;
        }

        f->Write(buffer, size);                                       // write the buffer to the OpenFile object                         
        ioLock->Release();
    }

    // printf("end write\n");
    delete [] buffer;
    return ;
}

//----------------------------------------------------------------------
//  SC_READ()
//   Implements the Reac() system call. Will attempt to read a certain number
//  of bytes from an OpenFile object or from ConsoleInput to a buffer
//  and then save that buffer back into memory.
//----------------------------------------------------------------------
int
SC_READ() {
    char *buffer = LoadStringFromMemory(machine->ReadRegister(4));     // grab buffer argument from register    
    if(buffer == NULL)     // error bad input
        return -1;

    int size = machine->ReadRegister(5);                              // grab number of bytes to read
    OpenFileId id = machine->ReadRegister(6);                         // grab file id of file to read from

    int numRead = 0;                                                 //initializing numRead to 0
    if(id == ConsoleInput) {                                        // if we are reading from ConsoleInput, read using synchConsole

        ioLock->Acquire();
        for(int i = 0; i < size; ++i, ++numRead)                    // read size amount of bytes from the Console
            buffer[i] = synchConsole->ReadChar();
        ioLock->Release();
    } 
    else {                                                            // else we are reading from a file
        ioLock->Acquire();
        OpenFile *f = currentThread->space->fileVector->Resolve(id);     // use the table of open files to resolve the fileid to an OpenFile object
        if(f == NULL) {                                                   // if we are trying to read from a bad fileid, signal error
            delete [] buffer;
            ioLock->Release();
            return -1;
        }

        numRead = f->Read(buffer, size);                              // read size amount of bytes from the file
        ioLock->Release();
    }

    SaveStringToMemory(buffer, numRead, machine->ReadRegister(4));      // write the characters read back to memory
    delete [] buffer;
    return numRead;
}

//----------------------------------------------------------------------
//  SC_EXEC()
//   Implements the Exec() system call, by loading a new addrspace
//   into memory and creating a forked thread - also updates the processlist
//   appropriately. Calls the VoidFunctionPtr StartProcess for the forked thread
//----------------------------------------------------------------------
SpaceId SC_EXEC(){
    char *filename = LoadStringFromMemory(machine->ReadRegister(4));
    int argVectorBase = machine->ReadRegister(5);        //Grabbing the arguments to the program
    int shareflag = machine->ReadRegister(6);           // grab shareflag

    if(filename == NULL)                                // could not load string from memory
        return -1;

    OpenFile *executable = fileSystem->Open(filename, currentThread->space->wdSector);  // create new open file object for the executable
    if(executable == NULL) {                            // could not open file
        // printf("\nUnable to open file: %s\n", filename);
        fflush(stdout);
        delete[] filename;
        return -1;
    }

    bool is_checkpoint = false;

    char buffer[13];
    memset(buffer, '\0', sizeof(buffer));
    executable->Read(buffer, 12);

    if(!strcmp(buffer, CHECKPOINT_TAG)) 
        is_checkpoint = true;


    char **argStrings;                  // will have arguments loaded into
    if(argVectorBase == 0 || is_checkpoint)              // NULL passed in for argv
        argStrings = NULL;
    else {
        // memLock->Acquire();
        // printf("exec acquire\n");
        argStrings = new(std::nothrow) char*[11];                              // can only pass 11 argument
        for(int i = 0; i < 11; ++i) {                                           // iterate and load all argument strings into our char * array
            
            memLock->Acquire();
            int addr = currentThread->space->V2P(argVectorBase + (i * 4));       // get physical address of the virtual address of the start of the next argument string
            unsigned int vAddr = *(unsigned int *) &machine->mainMemory[addr];    // get virtual address of the start of the next argument string
            memLock->Release();

            if(vAddr == 0) {                  // no more arguments
                argStrings[i] = (char *) NULL;      // add terminating null pointer
                break;
            }

            argStrings[i] = LoadStringFromMemory(vAddr);                // load the corresponding argument string from memory
        }
        // printf("exec release\n");
        // memLock->Release();
    }


    AddrSpace *new_space = new(std::nothrow) AddrSpace(executable, is_checkpoint);         // build new address space for child process
    if(!new_space->GetSuccess()) {                                                 // child binary too big for memory, do not let run
        // printf("addrspace failed\n");
        delete[] filename;
        delete executable;
        delete new_space;
        return -1;              // signal exec failure
    }

    FileVector *newVector;                          // create new FileVector for the child process
    if(shareflag == 0 || is_checkpoint)                      
        newVector = new(std::nothrow) FileVector();         // if we are not sharing files, create a blank file vector
    else
        newVector = new(std::nothrow) FileVector(currentThread->space->fileVector);     // else inherit File Vector from parent

    new_space->fileVector = newVector;              // setup newly built FileVector

    int newProcessId = new_space->GetId();                                  // save process id of child process
    Thread *new_process = new(std::nothrow) Thread("forked thread");        // build new thread for process
    new_process->space = new_space;

    processList->Insert(new_space->GetId());     //inserting the current process into the processList

    new_process->Fork(StartProcess, (int) argStrings);                       // fork off new process

    // delete[] filename;        // no leaks
    delete executable;

    return newProcessId;   //temp spaceID
}

//----------------------------------------------------------------------
//  SC_JOIN()
//   Implements the Join() system call, waiting on the provided processId, 
//   returns the exit status of the desired process
//----------------------------------------------------------------------
int SC_JOIN(){
    SpaceId joinId = (SpaceId) machine->ReadRegister(4);              // grab the desired processId
    int joinStatus = processList->GetStatus(joinId);                  // get status of the process we wish to join on
    return joinStatus;
}

//----------------------------------------------------------------------
//  SC_EXIT()
//   Implements the Exit() system call, by signaling the processList data
//   structure with the exit status argument, then returns void. Any threads
//   waiting on this process to finish, get signalled by SetStatus
//----------------------------------------------------------------------
void
SC_EXIT() {
    // printf("start exit\n");
    int exitStatus = machine->ReadRegister(4);              // grab the exit status
    SpaceId currentProcessId = currentThread->space->GetId();     //grab the id of the current process
    processList->SetStatus(currentProcessId,exitStatus);    //signal any waiting processes

    memLock->Acquire();
    for(unsigned int i = 0; i < NumPhysPages; ++i) {
        if(reversePageTable[i] == currentThread->space) {
            reversePageTable[i] = NULL;
            break;
        }
    }

    // for(unsigned int i = 0; i < currentThread->space->GetNumPages(); ++i)
    //     diskMap->Clear(currentThread->space->sectorMap[i]);
    memLock->Release();

    // for(unsigned int i = 0; i < currentThread->space->GetNumPages(); ++i)
    //     diskMap->Clear(currentThread->space->sectorMap[i]);

    // printf("end exit\n");

    delete currentThread->space->fileVector;                //closing the file vector by deleting it
    return ;
}

int
SC_MAKEDIR(){
    char *filename = LoadStringFromMemory(machine->ReadRegister(4));     // grab filename argument from register
    if(filename == NULL)    // cant load filename string, so error
        return -1;

    DEBUG('a', "filename: %s\n", filename);
    if(!fileSystem->MakeDir(filename, 0, currentThread->space->wdSector)) {                    // attempt to create a new file
        delete [] filename;
        return -1;
    }
    delete [] filename;
    return 1;
}

int
SC_CHANGEDIR(){
    char *filename = LoadStringFromMemory(machine->ReadRegister(4));     // grab filename argument from register
    if(filename == NULL)    // cant load filename string, so error
        return -1;

    int sector;
    DEBUG('a', "filename: %s\n", filename);
    sector = fileSystem->ChangeDir(filename, currentThread->space->wdSector);                    // attempt to create a new file
    currentThread->space->wdSector = sector;

    delete [] filename;
    return sector;
}


int 
find_virtual_page(int physicalPage) {
    // for(unsigned int i = 0; i < currentThread->space->GetNumPages(); ++i) {         // find the virtual page which corresponds to our physical page victim
    //     if(currentThread->space->pageMap[i] == physicalPage)                 // found virtual page for physical page victim
    //         return i;
    // }

    // return -1;
    ASSERT(physicalPage >= 0 && physicalPage < NumPhysPages);
    if(reversePageTable[physicalPage] == NULL)
        return -1;

    unsigned int length = reversePageTable[physicalPage]->GetNumPages();
    int *pageMap = reversePageTable[physicalPage]->pageMap;
    for(unsigned int i = 0; i < length; ++i) {         // find the virtual page which corresponds to our physical page victim
        if(pageMap[i] == physicalPage)                 // found virtual page for physical page victim
            return i;
    }

    return -1;
}



#ifdef USE_TLB

//----------------------------------------------------------------------
// HandleTLBFault
//      Called on TLB fault. Note that this is not necessarily a page
//      fault. Referenced page may be in memory.
//
//      If free slot in TLB, fill in translation info for page referenced.
//
//      Otherwise, select TLB slot at random and overwrite with translation
//      info for page referenced.
//
//----------------------------------------------------------------------

void
HandleTLBFault(int vaddr)
{
    // printf("tlb fault\n");
    stats->numTLBFaults++;

    int vpn = vaddr / PageSize;
    if(vpn > (int) currentThread->space->GetNumPages() - 1) {            // if bad virtual address, force exit of process
        printf("bad virtual address\n");
        machine->WriteRegister(2, 1);                                // setup Exit() system call
        machine->WriteRegister(4, 1);                                // exit with value 1
        ExceptionHandler(SyscallException);                          // call Exit() handler
    }

    memLock->Acquire();
    if(currentThread->space->pageMap[vpn] == -1) {          // if page we are faulting for is on disk, load it into main memory
        stats->numPageFaults++;

        int victimPhysicalPage = Random() % NumPhysPages;  //replace w/alg like CLOCK
        int victimVirtualPage = find_virtual_page(victimPhysicalPage);

        if(victimVirtualPage != -1) {
            for(int i = 0; i < TLBSize; ++i) {                                              // check if new victim is in tlb
                if(machine->tlb[i].valid && machine->tlb[i].virtualPage == victimVirtualPage) {         // new victim is in tlb
                    machine->tlb[i].valid = false;                  // invalidate tlb entry
                    break;
                }
            }

            int sector = reversePageTable[victimPhysicalPage]->sectorMap[victimVirtualPage];
            // vmDisk->WriteSector(sector, &machine->mainMemory[victimPhysicalPage * PageSize]);
            vmFile->WriteAt(&machine->mainMemory[victimPhysicalPage * PageSize], SectorSize, sector * SectorSize);
            reversePageTable[victimPhysicalPage]->pageMap[victimVirtualPage] = -1;
        }

        // vmDisk->ReadSector(currentThread->space->sectorMap[vpn], &machine->mainMemory[victimPhysicalPage * PageSize]);
        vmFile->ReadAt(&machine->mainMemory[victimPhysicalPage * PageSize], SectorSize, currentThread->space->sectorMap[vpn] * SectorSize);
        currentThread->space->pageMap[vpn] = victimPhysicalPage;            // update mapping of our virtual page we loaded in
        reversePageTable[victimPhysicalPage] = currentThread->space; 
    }

    int victim = Random() % TLBSize;
    machine->tlb[victim].virtualPage = vpn;
    machine->tlb[victim].physicalPage = currentThread->space->pageMap[vpn]; // use page map to resolve vpn to physpagenum
    machine->tlb[victim].valid = true;
    machine->tlb[victim].dirty = false;
    machine->tlb[victim].use = false;
    machine->tlb[victim].readOnly = false;
    memLock->Release();
    // printf("end tlb fault\n");
}

#endif

//----------------------------------------------------------------------
// ExceptionHandler
//  Entry point into the Nachos kernel.  Called when a user program
//  is executing, and either does a syscall, or generates an addressing
//  or arithmetic exception.
//
//  For system calls, the following is the calling convention:
//
//  system call code -- r2
//      arg1 -- r4
//      arg2 -- r5
//      arg3 -- r6
//      arg4 -- r7
//
//  The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//  "which" is the kind of exception.  The list of possible exceptions 
//  are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    switch (which) {
        case SyscallException:
            
            switch (type) {
                case SC_Halt:
                    DEBUG('a', "Shutdown, initiated by user program.\n");
                    interrupt->Halt();
                case SC_Exit: {
                    SC_EXIT();
                    //#########################need to cleanup thread########################
                    currentThread->Finish();
                    return ;
                }
                case SC_Exec: {
                    SpaceId id = SC_EXEC();
                    machine->WriteRegister(2, id);                                     // store return value in register 2
                    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg)); //increment pc to next intruction 
                    return;  
                }                
                case SC_Join: {
                    int exitStatus = SC_JOIN();
                    machine->WriteRegister(2, exitStatus);                                     // store return value in register 2
                    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg)); //increment pc to next intruction
                    return;
                }
                case SC_Create:
                    SC_CREATE();
                    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));        // increment pc to next intruction
                    return;
                case SC_Open: {
                    OpenFileId id = SC_OPEN();
                    machine->WriteRegister(2, id);                                     // store return value in register 2
                    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg)); //increment pc to next intruction
                    return;
                }
                case SC_Read: {
                    int numRead = SC_READ();
                    machine->WriteRegister(2, numRead);
                    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));        // increment pc to next intruction
                    return ;
                }
                case SC_Write:
                    SC_WRITE();
                    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg)); //increment pc to next intruction
                    return;
                case SC_Close:
                    SC_CLOSE();
                    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));        // increment pc to next intruction
                    return;
                case SC_MakeDir: {
                    int result = SC_MAKEDIR();
                    machine->WriteRegister(2, result);
                    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));        // increment pc to next intruction
                    return;
                }
                case SC_ChangeDir: {
                    int result = SC_CHANGEDIR();
                    machine->WriteRegister(2, result);
                    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));        // increment pc to next intruction
                    return;
                }
                // case SC_CheckPoint: {
                //     int result = SC_CHECKPOINT();
                //     machine->WriteRegister(2, result);
                //     // printf("pc reg: %d\n", machine->ReadRegister(PCReg));
                //     machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
                //     // printf("pc reg: %d\n", machine->ReadRegister(PCReg));
                //     return ;
                // }
                // case SC_Fork:
                //     break;
                // case SC_Yield:
                //     break;    
                default:
                    printf("Undefined SYSCALL %d\n", type);
                    ASSERT(false);
            }

#ifdef USE_TLB
        case PageFaultException:
            HandleTLBFault(machine->ReadRegister(BadVAddrReg));
            break;
#endif
        default: ;
    }
}







#else

// exception.cc 
//  Entry point into the Nachos kernel from user programs.
//  There are two kinds of things that can cause control to
//  transfer back to here from user code:
//
//  syscall -- The user code explicitly requests to call a procedure
//  in the Nachos kernel.  Right now, the only function we support is
//  "Halt".
//
//  exceptions -- The user code does something that the CPU can't handle.
//  For instance, accessing memory that doesn't exist, arithmetic errors,
//  etc.  
//
//  Interrupts (which can also cause control to transfer from user
//  code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

#ifdef USE_TLB

//----------------------------------------------------------------------
// HandleTLBFault
//      Called on TLB fault. Note that this is not necessarily a page
//      fault. Referenced page may be in memory.
//
//      If free slot in TLB, fill in translation info for page referenced.
//
//      Otherwise, select TLB slot at random and overwrite with translation
//      info for page referenced.
//
//----------------------------------------------------------------------

void
HandleTLBFault(int vaddr)
{
  int vpn = vaddr / PageSize;
  int victim = Random() % TLBSize;
  int i;

  stats->numTLBFaults++;

  // First, see if free TLB slot
  for (i=0; i<TLBSize; i++)
    if (machine->tlb[i].valid == false) {
      victim = i;
      break;
    }

  // Otherwise clobber random slot in TLB

  machine->tlb[victim].virtualPage = vpn;
  machine->tlb[victim].physicalPage = vpn; // Explicitly assumes 1-1 mapping
  machine->tlb[victim].valid = true;
  machine->tlb[victim].dirty = false;
  machine->tlb[victim].use = false;
  machine->tlb[victim].readOnly = false;
}

#endif

//----------------------------------------------------------------------
// ExceptionHandler
//  Entry point into the Nachos kernel.  Called when a user program
//  is executing, and either does a syscall, or generates an addressing
//  or arithmetic exception.
//
//  For system calls, the following is the calling convention:
//
//  system call code -- r2
//    arg1 -- r4
//    arg2 -- r5
//    arg3 -- r6
//    arg4 -- r7
//
//  The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//  "which" is the kind of exception.  The list of possible exceptions 
//  are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    switch (which) {
      case SyscallException:
  switch (type) {
    case SC_Halt:
            DEBUG('a', "Shutdown, initiated by user program.\n");
            interrupt->Halt();
          default:
      printf("Undefined SYSCALL %d\n", type);
      ASSERT(false);
  }
#ifdef USE_TLB
      case PageFaultException:
  HandleTLBFault(machine->ReadRegister(BadVAddrReg));
  break;
#endif
      default: ;
    }
}

#endif // CHANGED

