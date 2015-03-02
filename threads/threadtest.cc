#ifdef CHANGED

#include "prodcons.h"
#include "system.h"
#include <string>

/*
static BufferManager *manager;						//The BufferManager used to test prodcons


//----------------------------------------------------------------------
//	Producer
//	 Add a new character from the shared string "Hello world" to
//	the buffer and print out the write operation
//----------------------------------------------------------------------
void
Producer(int which) {
	const char *message = "Hello world";		//The string used to fill the buffer between producers and consumers
	while(*message != '\0' ) {
		manager->Write(*message);
		++message;
	}
}

//----------------------------------------------------------------------
//	Consumer
//	 Read a character from the shared Buffer and print the read character
//	out to the terminal.
//----------------------------------------------------------------------
void 
Consumer(int which) {
	char read;

	while((read = manager->Read()) != '\0') {
	}
}


//----------------------------------------------------------------------
//	ProducerConsumerTest
//	Used to test implementation of producer and consumer, takes in parameters
//	for the number of producers, number of consumers, and buffersize from the 
//	command line. Initializes global BufferManager manager to 
//	be used by the producers/consumers appropriately.
//----------------------------------------------------------------------

void ProducerConsumerTest(int producers, int consumers, int buffersize){
	manager = new(std::nothrow) BufferManager("buffer", buffersize);
	int i, j;

	for(i = 0; i < producers; i++){		//Creating producers 
		Thread *producer = new(std::nothrow) Thread((std::string("Producer ") + char(i+1)).c_str());
		producer->Fork(Producer, i);
	}

	for(j = 0; j < consumers; j++){		//Creating consumers
		Thread *consumer = new(std::nothrow) Thread((std::string("Consumer ") + char(i+1)).c_str());
		consumer->Fork(Consumer, j);
	}
}
*/
#else


// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");

    Thread *t = new(std::nothrow) Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

#endif
