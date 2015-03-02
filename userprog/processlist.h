#ifdef CHANGED

#ifndef PROCESSLIST_H
#define PROCESSLIST_H

#include "copyright.h"
#include "list.h"
#include "synch.h"
#include "synchlist.h"

#include <new>


//ProcessEntry contains the semaphore and spaceId, used as entries
// in the ProcessList

class ProcessEntry{
	public:

		ProcessEntry();
		~ProcessEntry();

		int GetStatus();			// return exit status of Process
		void SetStatus(int status);		// set exit status of ProcessEntry

		SpaceId GetId();				// get SpaceId of process entry

		void Wait();					// If an Exit status has been specified, do nothing, else wait
		void Signal();					// signal that an exit status has been specified for this entry
                	
	private:
		Semaphore *s;			// semaphore used to signal when a process has set its exit status
		SpaceId processId;
		int exitStatus;		//sent back to threads calling Join	
};


// The following class defines a "process list" which is used to 
// keep track of the current processes on the system. This class is
// used in implementing multiprogramming. Upon creation of a new process,
// a node containing the SpaceId, status of process, and a semaphore specific
// to that SpaceId is appeneded to the process list. Upon termination of a process,
// the ProcessList signals the appropriate semaphore, waking any other processes
// that are waiting (such as in a Join)

class ProcessList {
	public:

		ProcessList();		// initialize a synchronized list
		~ProcessList();		// de-allocate a synchronized list

		void Insert(SpaceId id);	// append item to the end of the list,
					                // and wake up any thread waiting in remove

		ProcessEntry *Find(SpaceId id);		// returns the corresponding process to the spaceid

		int GetStatus(SpaceId id);	//get status of a specific process
		void SetStatus(SpaceId id, int status);	//set status of process

	private:
		Lock *lock;
		SynchList *processList;			// synchlist keeping track of processes in the system
};


#endif // PROCESSLIST_H

#endif // CHANGED
