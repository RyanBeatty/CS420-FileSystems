#ifdef CHANGED
// progtest.cc 
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.  
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "synchconsole.h"
#include "addrspace.h"
#include "synch.h"

#include <new>

// SynchConsole *synchConsole;

//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------

void
StartProcess(char *filename)
{
    OpenFile *executable = fileSystem->Open(filename, 1);
    AddrSpace *space;
    
    if (executable == NULL) {
	printf("Unable to open file %s\n", filename);
	return;
    }
    space = new(std::nothrow) AddrSpace(executable);
    if(!space->GetSuccess()) {                          // binary too big for memory
        delete space;
        delete executable;
        interrupt->Halt();                              // do not let binary run, kill machine
        return ;
    }

    FileVector *vector = new(std::nothrow) FileVector();
    space->fileVector = vector;

    currentThread->space = space;

    delete executable;			// close file

    

    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register

    timeSlicer = new Timer(TimeSlicerInterruptHandler, 0, false);      // initialize timer that will allow for time slicing
                                                                       // between mutliple user programs
    // printf("\nStartProcess new Id: %d: filename:%s",space->GetId(), filename);
    processList->Insert(space->GetId());     //inserting the current process into the processList

    machine->Run();			// jump to the user progam
    ASSERT(false);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int) { readAvail->V(); }
static void WriteDone(int) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void 
ConsoleTest (char *in, char *out)
{
    char ch;

    console = new(std::nothrow) Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new(std::nothrow) Semaphore("read avail", 0);
    writeDone = new(std::nothrow) Semaphore("write done", 0);
    
    for (;;) {
	readAvail->P();		// wait for character to arrive
	ch = console->GetChar();
	console->PutChar(ch);	// echo it!
	writeDone->P() ;        // wait for write to finish
	if (ch == 'q') return;  // if q, quit
    }
}
#else











// progtest.cc 
//  Test routines for demonstrating that Nachos can load
//  a user program and execute it.  
//
//  Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "synchconsole.h"
#include "addrspace.h"
#include "synch.h"
#include <new>

//----------------------------------------------------------------------
// StartProcess
//  Run a user program.  Open the executable, load it into
//  memory, and jump to it.
//----------------------------------------------------------------------

void
StartProcess(char *filename)
{
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;

    if (executable == NULL) {
    printf("Unable to open file %s\n", filename);
    return;
    }
    space = new(std::nothrow) AddrSpace(executable);    
    currentThread->space = space;

    delete executable;          // close file

    space->InitRegisters();     // set the initial register values
    space->RestoreState();      // load page table register

    machine->Run();         // jump to the user progam
    ASSERT(false);          // machine->Run never returns;
                    // the address space exits
                    // by doing the syscall "exit"

    synchConsole = new(std::nothrow) SynchConsole(NULL, NULL);
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
//  Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int) { readAvail->V(); }
static void WriteDone(int) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
//  Test the console by echoing characters typed at the input onto
//  the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void 
ConsoleTest (char *in, char *out)
{
    char ch;

    console = new(std::nothrow) Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new(std::nothrow) Semaphore("read avail", 0);
    writeDone = new(std::nothrow) Semaphore("write done", 0);
    
    for (;;) {
    readAvail->P();     // wait for character to arrive
    ch = console->GetChar();
    console->PutChar(ch);   // echo it!
    writeDone->P() ;        // wait for write to finish
    if (ch == 'q') return;  // if q, quit
    }
}



#endif  // CHANGED
