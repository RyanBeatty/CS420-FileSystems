//    prodcons.h
//		defines interface for producer/consumer relationship. A BufferManager 
//		controls access to a shared Buffer object which can be written to/read from. 
//		Locks and Conditions are used when accessing the Buffer in order to provide
//		mutual exclusion and data overflow/underflow protextion for the Buffer.
//
//		A Buffer object can be of any size >= 1, and has indexes for the current
//		read and write positions in the Buffer.
//
//		A BufferManager should be able to manage any combination of Producers and Consumers
//		That want to access the Buffer.
#ifdef CHANGED

#ifndef PRODCONS_H
#define PRODCONS_H

#include "synch.h"
#include <new>

#define EMPTY_BUFFER_SLOT '-'		// when printing the state of the buffer, a '-' signifies an empty slot

class Buffer {
	public:
  		char *buffer;		 //char array
		int length;          //length of the buffer
		Buffer(int n);       //set initial buffer to size n
		~Buffer();			 //destroy buffer

		void PrintBuffer();  // print state of the buffer
		void FillBuffer(char c);		// fills the entire buffer with c
};

class BufferManager {
	public:
		Lock *lock;					//mutual exclusion
		Condition *full, *empty;
		Buffer *buf;				//buffer being managed
		BufferManager(const char* debugName, int length);	//set initial buffer manager
		~BufferManager();			//destroy buffer manager

		void Write(char c);			//method for writing chars into buffer
		char Read();				//method for reading chars from buffer
	private:
		const char *name;
		int rPos, wPos;		 //read/write index for the buffer
};



#endif
#endif


