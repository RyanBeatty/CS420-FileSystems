// alarm.h 
//	This class is used to implement an alarm clock that wakes threads after a given time period.
//	Threads call Alarm::GoToSleepFor(int howLong) to be added to the list of waiting threads. The Alarm
// 	Clock uses a timer to detect interrupts during a given time period, decrement each time on the list by 100
//	and wake the appropriate ones.

#ifdef CHANGED

#ifndef ALARM_H
#define ALARM_H

#include "synch.h"
#include "system.h"
#include <math.h>

void AlarmHandler(int a);	//The handler for the Timer

//The class used in creating the queue of threads, contains the thread reference and the amount of time the thread wants to wait
class AlarmQueueItem{
	public:
		AlarmQueueItem(Thread *thread, int duration);
		Thread *alarmThread;
		int timeRemaining;
};

//The class used to implement the timer and alarm, threads queue up on the Alarm object and wait 
class Alarm{
	public:
		Alarm(const char *debugName);				//Initializing the Alarm object
		~Alarm();							//Destroying the Alarm object
		void GoToSleepFor(int howLong);		//The method called by the current thread to go to sleep for a certain amount of time

		const char *name;					//debug name of Alarm
		int curTime;						// keeps track of time waited

		List *queue;						//The list of threads waiting on the alarm clock
		Lock *lock;							//Mutual exclusion for access to the list
		Timer *timer; 						//The timer used by the alarm clock
};

#endif	//ALARM_H
#endif	
