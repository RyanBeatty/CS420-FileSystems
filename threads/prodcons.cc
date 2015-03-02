/*
prodcons.cc

Implementation of a producer consumer relationship

Issues:
Buffer Size 1 does not work
*/


#ifdef CHANGED

#include "prodcons.h"
#include <string>


//----------------------------------------------------------------------
// Buffer::Buffer
// 	Initialize a buffer, so that it can be used for prodcons.
//
//	"n" is the length of the buffer.
//----------------------------------------------------------------------
Buffer::Buffer(int n) {
    buffer = new(std::nothrow) char[n];
    length = n;
}

//----------------------------------------------------------------------
// Buffer::~Buffer
// 	De-allocate buffer, when no longer needed.
//----------------------------------------------------------------------
Buffer::~Buffer() {
	delete buffer;
}

//----------------------------------------------------------------------
// Buffer::PrintBuffer
// 	Prints the current state of the buffer
//----------------------------------------------------------------------
void 
Buffer::PrintBuffer() {
	printf("\"");
	for (int i = 0; i < length; i++)
		printf("%c", buffer[i]);
	printf("\"");
	printf("\n");
}

//----------------------------------------------------------------------
// Buffer::FillBuffer
//	fills entire contents of buffer with c
//----------------------------------------------------------------------
void
Buffer::FillBuffer(char c) {
	for(int i = 0; i < length; ++i)
		buffer[i] = c;
}

//----------------------------------------------------------------------
// BufferManager::BufferManager
// 	Initialize a BufferManager, so that it can be used for prodcons.
//
//	"length" is the length of the buffer.
//----------------------------------------------------------------------
BufferManager::BufferManager(const char *debugName, int length) {
	buf = new(std::nothrow) Buffer(length);   //initialize new buffer
	lock = new(std::nothrow) Lock((std::string(debugName) + std::string(": Lock")).c_str());					//initialize lock with debugName + ": Lock"
	full = new(std::nothrow) Condition((std::string(debugName) + std::string(": Condition full")).c_str());		//initialize condition with debugName + ": Condition full"
	empty = new(std::nothrow) Condition((std::string(debugName) + std::string(": Condition empty")).c_str());	//initialize condition with debugName + ": Condition empty"
	
	name = debugName;
	rPos = wPos = 0;

	// sets all spaces in buffer to be "empty"
	// when printing the buffer, a '_' character denotes an empty buffer space
	buf->FillBuffer(EMPTY_BUFFER_SLOT);
}

//----------------------------------------------------------------------
// BufferManager::~BufferManager
// 	De-allocate buffer, lock, full, empty when no longer needed.
//----------------------------------------------------------------------
BufferManager::~BufferManager() {
	delete buf; delete lock; delete full; delete empty;
}

//----------------------------------------------------------------------
// BufferManager::Write
// 	Write a character into the buf.
//
//  Aquire lock for mutex. Check write index position. If write position is 
//  next to read position, buffer is full and we wait on Condition empty while full.
//  When there is space, we put char c in buf, increment our write position,
//	signal Condition full that there is content to read and then release the lock.
//----------------------------------------------------------------------
void
BufferManager::Write(char c) {
	lock->Acquire();    		//Aquire lock for mutex
	while((wPos + 1) % buf->length == rPos) {   //Wait while buffer is full
		empty->Wait(lock);
	}

	buf->buffer[wPos] = c;   						  //Write char into buf at the writing position
	wPos = (wPos + 1) % buf->length;            //Increment writing position, back to 0 if index is length of buffer

	printf("%s: Wrote '%c' to Buffer. STATE OF BUFFER: ", name, c);   //Print writing operation
	buf->PrintBuffer();     //Print current state of the buffer
	fflush(stdout);

	full->Signal(lock);     //Signal there is content to read
	lock->Release();		//Release mutex lock
}	

//----------------------------------------------------------------------
// BufferManager::Read
// 	Read a character at read position from buf.
//
//  Aquire lock for mutex. Check read index position. 
//	If read position equals write position, buffer is empty and we wait on Condition full. 
//	When there is content to read, we read a character, "clear" that position in the buffer by setting it
//	to '_', increment our next read position, signal that there is space to write, and
//  then release the lock.
//----------------------------------------------------------------------
char
BufferManager::Read() {
	lock->Acquire();    		//Aquire lock for mutex
	while(rPos == wPos) {    //Wait while buffer is empty
		full->Wait(lock);
	}

	char result = buf->buffer[rPos];				// read character from buffer
	buf->buffer[rPos] = EMPTY_BUFFER_SLOT;		// set index to '_', signifying "empty" buffer space
	rPos = (rPos + 1) % buf->length;		// increment next read index

	printf("%s: Read '%c' from Buffer. STATE OF BUFFER: ", name, result);   //Print reading operation
	buf->PrintBuffer();				// print current buffer status
	fflush(stdout);

	empty->Signal(lock);	// signal that there is space in the buffer
	lock->Release();		// release lock
	return result;
}

#endif