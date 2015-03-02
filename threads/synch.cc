// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(const char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new(std::nothrow) List;
}

//----------------------------------------------------------------------
// Semaphore::~Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!


//----------------------------------------------------------------------
// Lock::Lock
//  Constructor for Lock object
//  sets lock name to debugName, initializes waiting queue, and
//  initializes reference to holdingThread as null
//----------------------------------------------------------------------
Lock::Lock(const char* debugName) {
#ifdef CHANGED
    state = FREE;
    name = debugName;
    holdingThread = NULL;
    queue = new(std::nothrow) List;
#endif
}

//----------------------------------------------------------------------
// Lock::~Lock
//  cleanup allocated memory. delete reference to waiting queue
//----------------------------------------------------------------------
Lock::~Lock() {
#ifdef CHANGED
    delete queue;
#endif
}

//----------------------------------------------------------------------
// Lock::Acquire
//  Tries to gain access to the lock. Waits if lock is not free.
//  Turns interrupts off and waits until lock is free. Then
//  sets lock to busy and restores interrupts to previous state
//----------------------------------------------------------------------
void 
Lock::Acquire() {
#ifdef CHANGED
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // turn interrupts off

    while(state == BUSY) {                      // lock not free
        queue->Append((void *) currentThread);   // go to sleep
        currentThread->Sleep();
    }

    state = BUSY;                               // acquire lock
    holdingThread = currentThread;              // save reference to acquiring thread
    (void) interrupt->SetLevel(oldLevel);       // restore interrupt level
#endif
}

//----------------------------------------------------------------------
// Lock::Release
//  Turn interrupts off, frees lock, and wakes next waiting thread.
//----------------------------------------------------------------------
void 
Lock::Release() {
#ifdef CHANGED
    Thread *nextThread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // turn interrupts off

    // only release lock if owner of lock
    ASSERT(isHeldByCurrentThread());

    nextThread = (Thread *) queue->Remove();
    if(nextThread != NULL)                              // make thread ready
        scheduler->ReadyToRun(nextThread);
    
    state = FREE;                               // free lock
    holdingThread = NULL;                       // clear holdingThread

    (void) interrupt->SetLevel(oldLevel);       // restore interrupts
#endif
}


//----------------------------------------------------------------------
// Lock::isHeldByCurrentThread
//  Returns True if currentThread holds the lock and false otherwise
//----------------------------------------------------------------------
bool
Lock::isHeldByCurrentThread() {
#ifdef CHANGED    
    return currentThread == holdingThread;
#endif
}

#ifdef CHANGED

//----------------------------------------------------------------------
// Condition::Condition
//  Instantiate a condition variable, that allows threads to queue and 
//  wait for a signal. The condition variable contains a queue of waiting
//  threads, and a debugName char*
//----------------------------------------------------------------------
Condition::Condition(const char* debugName) { 
    name = debugName;
    queue = new(std::nothrow) List;
}

//----------------------------------------------------------------------
// Condition::~Condition
//  Clean up memory and remove the queue, destroying the Condition object
//----------------------------------------------------------------------
Condition::~Condition() { 
    delete queue;
}

//----------------------------------------------------------------------
// Condition::Wait
//  Directs the current thread to wait on the condition variable until
//  signalled. First, release conditionLock and put the current thread to
//  sleep. Upon waking, reacquire the lock released earlier.
//----------------------------------------------------------------------
void Condition::Wait(Lock* conditionLock) { 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);  //turn off interrupts
    conditionLock->Release();                       //release the conditionLock if held

    queue->Append((void *)currentThread);           //add thread to queue
    currentThread->Sleep();                         //put current thread to sleep

    conditionLock->Acquire();                       //reacquire conditionLock
    (void) interrupt->SetLevel(oldLevel);           //turn interrupts on
}

//----------------------------------------------------------------------
// Condition::Signal
//  Wakes a thread from the queue, but can only execute if the currentThread 
//  holds the Lock. Remove thread from queue, set ready to run and release Lock.
//----------------------------------------------------------------------
void Condition::Signal(Lock* conditionLock) { 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);  //turn off interrupts
    ASSERT(conditionLock->isHeldByCurrentThread());             //Ensuring the current thread holds the lock
    
    Thread *thread = (Thread *)queue->Remove();                 //Removing thread from queue
    if(thread != NULL)
        scheduler->ReadyToRun(thread);

    (void) interrupt->SetLevel(oldLevel);           //turn interrupts on
}

//----------------------------------------------------------------------
// Condition::Broadcast
//  Wakes up all the queued threads 
//----------------------------------------------------------------------
void Condition::Broadcast(Lock* conditionLock) { 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);  //turn off interrupts
    ASSERT(conditionLock->isHeldByCurrentThread());             //Ensuring the current thread holds the lock
    
    Thread *nextThread = (Thread *) queue->Remove();
    
    while(nextThread != NULL){
        scheduler->ReadyToRun(nextThread);              //schedule thread to run
        nextThread = (Thread *) queue->Remove();        //remove the thread from the queue
    }

    (void) interrupt->SetLevel(oldLevel);           //turn interrupts on
}

#else
Condition::Condition(const char* debugName) { }
Condition::~Condition() { }
void Condition::Wait(Lock* conditionLock) { ASSERT(false); }
void Condition::Signal(Lock* conditionLock) { }
void Condition::Broadcast(Lock* conditionLock) { }
#endif
