// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
#ifdef CHANGED

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
#include <new>

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

#ifdef USER_PROGRAM
#include "machine.h"
#include "synchconsole.h"
#include "bitmap.h"
#include "processlist.h"
#include "filetable.h"

extern Machine* machine;						// user program memory and registers
extern SynchConsole *synchConsole;				 // synchconsole class
extern BitMap *bitMap;							//manages allocation of pages
extern Timer *timeSlicer;						 // timer that will allow time slicing
extern void TimeSlicerInterruptHandler(int);
extern ProcessList *processList;	//list of processes

extern SpaceId processId;
extern Lock *ioLock;

extern OpenFileTable *globalFileTable;
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
#include "bitmap.h"
extern SynchDisk   *synchDisk;
extern Lock *directoryLock;
extern Lock *diskmapLock;
extern Lock *diskLock;
extern OpenFile *vmFile;

extern BitMap *diskMap;			// bitmap for allocating disk sectors
// extern SynchDisk *vmDisk;	// our disk for secondary storage
extern AddrSpace *reversePageTable[];
extern Lock *memLock;

#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif


#endif // SYSTEM_H





#else

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
#include <new>

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

#ifdef USER_PROGRAM
#include "machine.h"
#include "processlist.h"

extern Machine *machine;	// user program memory and registers
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif

#endif // SYSTEM_H

#endif // CHANGED