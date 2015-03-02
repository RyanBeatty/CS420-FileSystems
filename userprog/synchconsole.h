// synchconsole.h
// 	Offers support for synchronous interface to the console

#ifdef CHANGED
#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

#include "console.h"
#include "synch.h"
#include <new>

// Similar to the Disk, the Console supports asynchronous I/O with interrupts.
// This class allows the console to be implemented synchronously, with only a single 
// thread reading or writing to the console at a given.  
class SynchConsole{
	public:
		SynchConsole(char* in, char* out);		//Creates an instance of synchconsole with references to the in and out file descriptors, as char*
		~SynchConsole();						//deallocate and remove this instance of SynchConsole

		char ReadChar();						//Read in from the stdin of the console
		void WriteChar(char a);				//Write out to the stdin of the console

		void ReadAvail();				//The handler argument to Console called when there are characters to be read
		void WriteDone();				//The handler argument to Console called when all characters have been written

	private:
		Console *console;						//instance of Console
		Lock *readLock;							//Lock used for reading from stdin
		Lock *writeLock; 						//Lock used to write to stdin
		Semaphore *rSem;					//semaphore used to signal from void handler function
		Semaphore *wSem; 
};


#endif // SYNCHCONSOLE_H
#endif // CHANGED