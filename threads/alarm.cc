//	alarm.cc
//
//	Used to implement our alarm clock based on the Timer.
//	An Alarm manages Threads who wish to sleep for a certain number of Ticks.
//	The Alarm has its own Timer, and every Timer interrupt, the Alarm checks
//	for any Threads that need to woken up.

#ifdef CHANGED
#include "alarm.h"


//----------------------------------------------------------------------
// AlarmQueueItem::AlarmQueueItem
//	The item that will be stored in the queue of waiting threads within our Alarm clock.
//	Stores the current thread, and the amount of time remaining
//----------------------------------------------------------------------
AlarmQueueItem::AlarmQueueItem(Thread *thread, int duration){
	alarmThread = thread;
	timeRemaining = duration;
}


//----------------------------------------------------------------------
// Alarm::Alarm
//	Initialize the Alarm class, along with the objects that are necessary,
//	pass in the handler and start the Timer. 
//----------------------------------------------------------------------
Alarm::Alarm(const char* debugName){
	name = debugName;								//Set the debug name
	curTime = 0;								// no time has passed
	queue = new(std::nothrow) List;				//initializing the list
	lock = new(std::nothrow) Lock("alarm lock");	//alarm clock lock
	timer = new(std::nothrow) Timer(AlarmHandler, (int) this, false);	// new timer for the alarm clock
}

//----------------------------------------------------------------------
// Alarm::~Alarm
//	Remove instance of the alarm class, delete all the objects initialized with it
//----------------------------------------------------------------------
Alarm::~Alarm(){
	delete queue; delete lock; delete timer;
}

//----------------------------------------------------------------------
// Alarm::GoToSleepFor
//	Called by threads accessing the Alarm, which then proceed to go to sleep until
//	woken by the Alarm appropriately. Reference to the thread and amount of time needed to wait
//	are stored in an AlarmQueueItem and added to the list
//----------------------------------------------------------------------
void
Alarm::GoToSleepFor(int howLong) {
	IntStatus oldLevel = interrupt->SetLevel(IntOff);  //turn off interrupts

	int diff = howLong % TimerTicks;
	diff = diff == 0 ? 0 : 100 - diff;	// calc the difference to next time interval

	// print out timing statistics. used to show correctness of Alarm
	printf("Thread going to sleep at time: %d. ", curTime);
	printf("Sleeping for: %d. ", howLong);
	printf("Will wake at time: %d\n", curTime + howLong + diff);
	fflush(stdout);

	AlarmQueueItem *alarmItem = new AlarmQueueItem(currentThread, howLong);		//creating a new instance of alarmItem with a reference to currentThread
	queue->Append((void *) alarmItem);											//Appending our item to the queue
	currentThread->Sleep();														//put the current thread to sleep

	(void) interrupt->SetLevel(oldLevel);           //turn interrupts on
}

//----------------------------------------------------------------------
// AlarmHandler
//	Handle the interrupt from our timer, iterate over the queue, and wake the appropriate ones
//	Items are removed from the queue and appended back into a new queue if they aren't ready to be woken,
//	The new queue is finally set as the initial queue
//
//	"a" should be an Alarm pointer cast to an int
//----------------------------------------------------------------------
void
AlarmHandler(int a) {
	Alarm *alarm = (Alarm *) a;		// grab refernce to Alarm

	alarm->curTime += TimerTicks;		// "TimerTicks" amount of time has passed

	AlarmQueueItem *currentItem = (AlarmQueueItem *) alarm->queue->Remove();	// get first item off wait queue
	List *newQueue = new List();

	if(currentItem != NULL) {			// only print current time when there are sleepers to avoid constant printing to stdout
		printf("Current Alarm Time: %d\n", alarm->curTime);	// print out current Alarm time
		fflush(stdout);
	}

	while(currentItem != NULL){					// iterate over Alarm queue
		currentItem->timeRemaining -= TimerTicks;		// decrement each element's time remaining
		
		if(currentItem->timeRemaining <= 0){			// check if current item should be woken up
			scheduler->ReadyToRun(currentItem->alarmThread);	//waking the thread
			printf("Thread Woken Up At Time: %d\n", alarm->curTime);	// print wake time
			fflush(stdout);
		}
		else{										// if thread still sleeping, add back to waiting queue
			newQueue->Append((void *) currentItem);
		}

		currentItem = (AlarmQueueItem *) alarm->queue->Remove();	// grab next thread off queue
	}
	
	alarm->queue = newQueue;				//resetting the queue to be new queue
}


#endif