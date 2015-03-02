#ifdef CHANGED
//	alarmtest.cc
//
//	Implements tests for the Alarm Clock problem/class.
//	Sleeper Threads are spawned and tell the Alarm Clock how long they
//	want to sleep for. The Alarm Clock puts the threads asleep and then
//	checks if any Threads need to awoken every Timer interrupt (100 ticks).
//	Therefore, Threads sleep for the amount of ticks specified rounded to the
//	nearest hundred (i.e. if a Thread wants to sleep for 250 ticks, it will wake
//	when after 300 ticks).

#include "alarm.h"

Alarm *alarm;

//----------------------------------------------------------------------
// Sleeper
//	A Sleeper Thread goes to sleep on the Alarm for a given amount of
//	time until it is woken up
//
//	"howLong" - number of ticks that the Sleeper would like to sleep for
//----------------------------------------------------------------------
void
Sleeper(int howLong) {
	alarm->GoToSleepFor(howLong);
}


//----------------------------------------------------------------------
// AlarmTest
//	Testing function for the Alarm Clock.
//	A new, shared Alarm is created along with Sleeper Threads who whish
//	to sleep on the Alarm Clock.
//
//	NOTE: Program will not terminate even after all Sleeper Threads finish
//		  must exit with Control-C.
//----------------------------------------------------------------------
void
AlarmTest() {
	alarm = new(std::nothrow) Alarm("alarm");		// new Alarm created

	Thread *sleeper;
	for(int i = 100; i < 500; i += 25) {	// Create sleeper with varying sleep times 
		sleeper = new(std::nothrow) Thread("sleeper");
		sleeper->Fork(Sleeper, i);
	}
	Sleeper(100);
}
#endif // CHANGED