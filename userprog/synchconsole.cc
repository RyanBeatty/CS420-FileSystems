#ifdef CHANGED
#include "synchconsole.h"

// synchconsole.cc
//	implement routines to synchronously access the console

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------
static void
ConsoleReadAvail(int arg){ 
	SynchConsole* console = (SynchConsole *)arg;
    console->ReadAvail();
}

static void
ConsoleWriteDone(int arg) { 
	SynchConsole* console = (SynchConsole *)arg;
    console->WriteDone();
}

//----------------------------------------------------------------------
// SynchConsole::SynchConsole
// 	Initialize the synchronous interface to the console, in turn
//	initializing the normal console.
//
//	arguments are the file descriptors to be written and read from
//----------------------------------------------------------------------
SynchConsole::SynchConsole(char *in, char *out){
	readLock = new(std::nothrow) Lock("Console Read Lock");
	writeLock = new(std::nothrow) Lock("Console Write Lock");
	rSem = new(std::nothrow) Semaphore("read avail", 0);
	wSem = new(std::nothrow) Semaphore("write done", 0);
	console = new(std::nothrow) Console(in, out, ConsoleReadAvail, ConsoleWriteDone, (int) this);
}

//----------------------------------------------------------------------
// SynchConsole::~SynchConsole
// 	delete the SynchConsole and remove all objects
//----------------------------------------------------------------------
SynchConsole::~SynchConsole(){
	delete readLock;
	delete writeLock;
	delete rSem;
	delete wSem;
	delete console;
}


//----------------------------------------------------------------------
// SynchConsole::ReadChar()
//	Acquire the read semaphore, ensure mutex to console and read
//----------------------------------------------------------------------
char
SynchConsole::ReadChar(){
	rSem->P();
	readLock->Acquire();
	char c = console->GetChar();
	readLock->Release();
	return c;
}

//----------------------------------------------------------------------
// SynchConsole::WriteChar(char c)
//	Acquire the write semaphore, ensure mutex to console, an release once
//	console has finished writing
//----------------------------------------------------------------------
void
SynchConsole::WriteChar(char c){
	writeLock->Acquire();
	console->PutChar(c);		//writing out to stdout
	wSem->P();
	writeLock->Release();
}

void
SynchConsole::ReadAvail(){
	rSem->V();
}
void
SynchConsole::WriteDone(){
	wSem->V();
}
#endif // CHANGED