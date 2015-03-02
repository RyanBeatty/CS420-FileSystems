#ifdef CHANGED

// processlist.cc
//	Routines for synchronized access to a list.
//
//	Implemented by surrounding the List abstraction
//	with synchronization routines.
//
// 	Implemented in "monitor"-style -- surround each procedure with a
// 	lock acquire and release pair, using condition signal and wait for
// 	synchronization.
//
//	
//

#include "copyright.h"
#include "synchlist.h"
#include "processlist.h"



//----------------------------------------------------------------------
// ProcessEntry::ProcessEntry
//	Initialize the data structures required for the ProcessEntry
//----------------------------------------------------------------------

ProcessEntry::ProcessEntry()
{
    // processId = id;
    s = new(std::nothrow) Semaphore("exit signal sem",0);
    exitStatus = 0;
}

//----------------------------------------------------------------------
// ProcessEntry::~ProcessEntry
//	De-allocate the data structures created for synchronizing a process list. 
//----------------------------------------------------------------------

ProcessEntry::~ProcessEntry()
{ 
    delete s;
}

int
ProcessEntry::GetStatus() {
	return exitStatus;
}

void
ProcessEntry::SetStatus(int status) {
	exitStatus = status;
}

void
ProcessEntry::Wait() {
	s->P();
}

void
ProcessEntry::Signal() {
	s->V();
}

// SpaceId
// ProcessEntry::GetId() {
// 	return processId;
// }


//######################## BEGIN PROCESSLIST CODE ##########################



//----------------------------------------------------------------------
// ProcessList::ProcessList
//	Initialize the data structures required for the ProcessList
//----------------------------------------------------------------------
ProcessList::ProcessList()
{
	processList = new SynchList();
    lock = new(std::nothrow) Lock("processlist lock");
}

//----------------------------------------------------------------------
// ProcessList::~ProcessList
//	De-allocate the data structures created for synchronizing a process list. 
//----------------------------------------------------------------------

ProcessList::~ProcessList()
{ 
    delete processList;
    delete lock;
}

//----------------------------------------------------------------------
// ProcessList::Insert()
//	insert the corresponding process SpaceId into the list of processes
//----------------------------------------------------------------------
void
ProcessList::Insert(SpaceId id)
{
	ProcessEntry *item = new(std::nothrow) ProcessEntry();					// create a new entry for process list
	processList->SortedInsert((void *) item, (long long unsigned) id);		// insert the new entry using the SpaceId as a key for sorted insert
}

//----------------------------------------------------------------------
// ProcessList::GetStatus()
//	returns the exit status of the Process corresponding to the passed in
//	SpaceId. This method will wait until the Process actually Exits()
//----------------------------------------------------------------------
int 
ProcessList::GetStatus(SpaceId id){
	if(id < 0)							// if bad SpaceId, return failure
		return -1;

	ProcessEntry *entry = (ProcessEntry *) processList->SortedFind(id);		// find the corresponding Entry given the SpaceId
	if(entry == NULL)		// return failure if SpaceId not in list
		return -1;
	
	entry->Wait();							// wait until an exit value has been set for the process
	lock->Acquire();
	int exitStatus = entry->GetStatus();		// get the exit status of the process
	lock->Release();						
	entry->Signal();						// signal so that other processes can also get the exit status

	return exitStatus;
}

//----------------------------------------------------------------------
// ProcessList::SetStatus()
//	Sets the exit status of the Process with the passed in SpaceId and signals
//	that an exit status has been specified by this process
//----------------------------------------------------------------------
void
ProcessList::SetStatus(SpaceId id, int status) {

	ProcessEntry *entry = (ProcessEntry *) processList->SortedFind((long long unsigned) id);		// find ProcessEntry based of SpaceId

	if(entry == NULL)			// if bad SpaceId passed in, return 
		return ;
	
	lock->Acquire();
	entry->SetStatus(status);		// set the exit status for the Process
	lock->Release();
	entry->Signal();				// signal that this Process has Exited()
	return ;
}

#endif // CHANGED