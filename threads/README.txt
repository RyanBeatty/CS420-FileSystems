Nachos

CS444 Project, Fall 2014
Hareesh Nagaraj, Ryan Beatty, Joe Soultanis

Project 1
*************************************************
-------------------------------------------------
Problem 1: Lock and Condition Variable Design
-------------------------------------------------

-Most changes/additions inside synch.*

-Implemented Lock similar to a binary Semaphore, where Lock value is either 1 or 0 (with macros FREE or BUSY). Threads request access to lock, and queue up and sleep if not given access. These macros are saved in private variable "state".

-Pointer reference to thread with access to lock (called "holdingThread") maintained in order to implement isHeldByCurrentThread()

-Lock::Acquire(): Turn interrupts off. Check if Lock is BUSY. If Lock BUSY, add to wait queue and go to sleep, else grab Lock. Turn interrupts on.

-Lock::Release(): Turn off Interrupts. Check if currentThread is the holding thread (if it isn't there is a programming error so kill execution). Grab the next thread saved inside the private variable "queue" and use scheduler to indicate that the thread is ready to run. then turn interrupts on.

-Conditional variables implemented with queue property to allow threads to queue up and wait for a signal.

-Condition::Wait(): turn off interrupts and store the status of the level, release mutex Lock that was passed in as a parameter to the function, add our current thread to the condition variable's queue, Sleep the thread. Thread will stay in this position until signaled. When thread is signaled, it wakes up, requires the mutex lock, and turned interrupts back on with the level we had stored before.

-Condition::Signal(): turn off interrupts and store the status of the level, assert that the current thread has the mutex lock, remove the thread from the queue, set it as ready to run in our scheduler and turn interrupts back on. Wakes up threads that are sleeping from having Condition::Wait() called on them.

-Condition::Broadcast(): similar to signal...wakes up every queued thread. turn off interrupts and store the status of the level, assert that the current thread has the mutex lock, then, until there are no more available threads queued, we tell the scheduler that this thread is ready to run then pop another thread off the queue. Keep doing this until we run out of threads.


-------------------------------------------------
Problem 2: Producer/Consumer
-------------------------------------------------

-Most changes/additions inside prodcons.*, tests in threadtest.cc

-Buffer class contains "char *buffer" of size "int length" for writing to/reading from.

-Empty buffer slot defined to be '-'. Used for correctness printing.

-Buffer::PrintBuffer() loops through and prints the current state of the buffer

-Buffer::FillBuffer(char c) fills the buffer with char c. Used to initialize the buffer to all empty characters: '-'

-BufferManager uses mutex style locks and condition variables to control access to buffer. Lock implementation described above in "Problem 1: Lock Design"

-BufferManager maintains an integer index for the next read or write command, rPos and wPos, respectively. Whenever we read a char successfully, we increment rPos. Whenever we write a char successfully, we increment wPos. If rPos or wPos are ever equal to the length of the buffer, we us the mod operaion ("%") to loop that position index back to 0, the beginning of the buffer.

-BufferManager::Write(): If wPos is one position behind rPos and we try to write, we've determined that the buffer is full and wait using our condition variable "empty" until BufferManager::Read() is called, which signals empty after a read and opens up a spot in the buffer.

-BufferManager::Read(): If wPos and rPos are equal and we try to read, we've determined that the buffer is empty and wait on "full" until BufferManager::Write() is called, which will put content into the Buffer and signal on "full" that there is content to be read.

*** Current implementation will not work with buffer size 1 ***

-to test Producer/Consumer run nachos1 with: "./threads/nachos -P 2".
 You can also enable random time slicing with the call: "./threads/nachos -P 2 -rs <somenum>"
 You can also change the number of producers (as long as numProducers >= 1): "./threads/nachos -P 2 -pro <numProducers>"
 You can also change the number of consumers (as long as numConsumers >= 1): "./threads/nachos -P 2 -con <numConsumers>"
 You can also change the buffer size (as long as bufferSize > 1): "./threads/nachos -P 2 -buf <bufferSize>"

-------------------------------------------------
Problem 4: Alarm Clock
-------------------------------------------------

-Changes inside alarm.*, alarmtest.cc for testing

-Output description:

	--Will be printed when thread going to sleep:
		"Thread going to sleep at time: 100. Sleeping for: 100. Will wake at time: 200"

	--Will be printed when thread woken up:
		"Thread Woken Up At Time: 200"

	--Will be printed to indicate updated alarm time:
		"Current Alarm Time: 200"

-AlarmQueueItem class: Used to store info about the Threads that wish to sleep on the Alarm

-AlarmQueueItem objects will be appended into a queue defined in the Alarm class used to maintain sleeping alarm items. The class has public property timeRemaining which represents how long until the thread should be woken up. The constructor takes the thread and the duration (timeRemaining) to create the AlarmQueueItem.

-Alarm class contains properties "int curTime", "List *queue", and "Timer *timer". curTime is the total time waited, or amount of time passed in the Alarm, the queue is the list of threads waiting on the alarm clock to be woken up, and the timer is used by the alarm clock to regulate increments of 100 ticks

-Alarm::GoToSleepFor(int howLong): We turn interrupts off and put the current sleep request Thread to sleep for the given input duration "howLong", rounded up the the nearest hundreth. We determine the next increment of 100 to round to, construct a new AlarmQueueItem with the appropriate sleep time, append the item to a queue of sleeping alarms and then put the thread to sleep.When the Thread wakes, we turn interrupts back on.

-AlarmHandler(int a) is our handler function that is passed to the Alarm's Timer. It is responsible for waking up threads at the appropriate time. The timer fires a new interrupt every "TimerTicks" amount of time and goes into the AlarmHandler. The handler iterates through the queue of items inside the sleeping alarm queue and decreases their timeRemaining by the "TimerTicks" amount of time. If a Thread has waited long enough, we set the thread as ReadyToRun on the scheduler and wake up the thread. If the thread is not ready to wake up, we reappend the item onto a new list to sleep. After we have processed every alarm in the queue, we adjust the pointer for the alarm sleeping queue to the new one we've built. This is a simple solution to the problem of waking up some alarms and storing others. Building a new List from the old List saves us the trouble of having to iterate over a list, managing pointers and modifing the List in-place.

-Our Tests for the Alarm consists of spawning a bunch of Sleeper Threads that will wait on the Alarm with increasing wait times.

-to run the Alarm tests use the following command: "./threads/nachos -P 4 -rs <someNumber>" 
 you MUST have random time slicing enabled


******SAMPLE OUTPUT******
// run using "./threads/nachos -P 4 -rs 1000"

Thread going to sleep at time: 0. Sleeping for: 100. Will wake at time: 100
Thread going to sleep at time: 0. Sleeping for: 125. Will wake at time: 200
Thread going to sleep at time: 0. Sleeping for: 150. Will wake at time: 200
Thread going to sleep at time: 0. Sleeping for: 175. Will wake at time: 200
Current Alarm Time: 100
Thread Woken Up At Time: 100
Thread going to sleep at time: 100. Sleeping for: 200. Will wake at time: 300
Current Alarm Time: 200
Thread Woken Up At Time: 200
Thread Woken Up At Time: 200
Thread Woken Up At Time: 200
Thread going to sleep at time: 200. Sleeping for: 100. Will wake at time: 300
Thread going to sleep at time: 200. Sleeping for: 225. Will wake at time: 500
Thread going to sleep at time: 200. Sleeping for: 275. Will wake at time: 500
Thread going to sleep at time: 200. Sleeping for: 300. Will wake at time: 500
Thread going to sleep at time: 200. Sleeping for: 325. Will wake at time: 600
Thread going to sleep at time: 200. Sleeping for: 350. Will wake at time: 600
Current Alarm Time: 300
Thread Woken Up At Time: 300
Thread Woken Up At Time: 300
Thread going to sleep at time: 300. Sleeping for: 375. Will wake at time: 700
Thread going to sleep at time: 300. Sleeping for: 400. Will wake at time: 700
Thread going to sleep at time: 300. Sleeping for: 425. Will wake at time: 800
Thread going to sleep at time: 300. Sleeping for: 450. Will wake at time: 800
Thread going to sleep at time: 300. Sleeping for: 475. Will wake at time: 800
Current Alarm Time: 400
Thread going to sleep at time: 400. Sleeping for: 250. Will wake at time: 700
Current Alarm Time: 500
Thread Woken Up At Time: 500
Thread Woken Up At Time: 500
Thread Woken Up At Time: 500
Current Alarm Time: 600
Thread Woken Up At Time: 600
Thread Woken Up At Time: 600
Current Alarm Time: 700
Thread Woken Up At Time: 700
Thread Woken Up At Time: 700
Thread Woken Up At Time: 700
Current Alarm Time: 800
Thread Woken Up At Time: 800
Thread Woken Up At Time: 800
Thread Woken Up At Time: 800

ARGUMENT FOR CORRECTNESS:
	Examining the above output, we can follow the threads as they invoke the alarm, and proceed to be woken appropriately. The first four threads that invoke alarm, a single thread is set to be woken at time 100 and three others to be woken at time 200 of the Alarm as it increments. At alarm time 100, a single thread is woken, and at alarm time 200, the other three are woken. Using this method to observe the other threads, we can see a proper sleep/wake behavior.


-------------------------------------------------
Problem 5: Elevator
-------------------------------------------------

-elevator.* for ElevatorRequest and ElevatorManager, elevatortest.* for testing, Elevator thread and Student threads

elevator.*:

Elevator Algorithm:
	The Elevator should then service requests, prioritizing requests made first and picking up students who can be serviced on the way to the next request.

Request Algorithm:
	A request comes into the ElevatorManager and checks if it needs to be put on the request Queue.	A request must go onto the queue if 1. the elevator is not going in the same direction as the my direction. or 2. (atFloor, toFloor) of the request is not within range(curFloor, destination) of the Elevator. There is a Condition array with one Condition per floor. Each reqeust waits on its atFloor Condition in the Condition array until the Elevator picks it up. It then waits on its toFloor condition in the Condition array until the Elevator drops it off. 

	-enum ELEVATOR_DIRECTION defines elevator states of movement: STOPPED, GOING_UP, GOING_DOWN

	-class ElevatorRequest is used to define incoming requests to use the elevator. It contains public properties "atFloor", the floor the request is coming from, "toFloor", the floor the request is going to, and  "direction", the direction of the request.

	-MovedFloor(int m) is our handler function that uses the timer to signal moving of floors at 100 tick intervals. Dummy lock used that no other thread can own to ensure mutex and proper signaling.

	-ElevatorRequest::ElevatorRequest(int atF, int toF): set requests' atF and toF to our atFloor, toFloor, and destination class properties.

	-ElevatorManager::ElevatorManager(const char *debugName, int n): takes a total number of floors with "int n", starts elevator at floor 0 with direction STOPPED and destination NO_DESTINATION. "reqQueue" is a queue of elevator requests to be serviced in FIFO order. "timer" initializes a Timer with the defined MovedFloor method to be our handler to controlle rthe changing of floors on 100 tick intervals. "floors" is an array of condition variables with length "numFloors" that we use to represent each of the individual floors, used so that we can broadcast the elevator's location to waiting requests at corresponding floors. "request" is a condition that waits on a student request to use the elevator. "move" is a condition that waits for the handler to signal "move". "lock" is used to ensure mutex.

	-ElevatorManager::ArrivingGoingFromTo(int atFloor, int toFloor): used to simulate a Student requesting the elevator from a floor "atFloor" to a desination "toFloor". Creates a new ElevatorRequest object with those parameters. If the elevator is busy and moving in a direction opposite to the current request's desired direction or the request's desired destination is not in the range of the elevator's current destination, the request is placed on the request queue. However, if the elevator and request have the same direction and the request's destination is within the range of the elevator's destination, the request is not added to the queue since the request can be serviced on the way to the current destination. The Student then waits on the location in the condition array "floors" corresponding to its current location, "atFloor". Once the elevator broadcasts to that condition variable based on its current floor, the Student then boards the elevator if the elevator is going in its desired direction and the elevator's destination is in the range of the Student's desired direction. After boarding the elevator, the Student proceeds on the wait on the variable in the condition array corresponding to "toFloor", the Student's desired destination. Once the Student is signalled to get off by the elevator's broadcast at "toFloor", the queue of requests needs to be updated to remove any redundancies that may have occured from other Students dropped off along the way. 

	-ElevatorManager::IsInRange(int atFloor, int toFloor): uses the elevator's current motion to determine if a given pair of "atFloor" and "toFloor" locations in a request are in range of the current destination. If the elevator is STOPPED, we automatically return false.

elevatortest.cc:

	-struct Student: a struct used to represent students representing their arrival and destination request

	-Student *GetStudent(int at, int to): method to return a Student struct with the given "atFloor" and "toFloor" based on parameters "at" and "to". Simplifies implementation of testing cases

	-Request(int which): used to simulate the request of the Student. Takes in an integer which is then cast to a student struct.

	-SetNextRequest(): Dequeues a request from the ElevatorRequest "reqQueue", sets the destination equal to the "curRequest->atFloor". Determines the direction and sets next direction. If our nextDirection is STOPPED, we set our ElevatorRequest direction to STOPPED. If our nextDirection is GOING_UP, we set our ElevatorRequest direction to GOING_UP. Same thing for GOING_DOWN.

	-Elevator(int which): Simulates the actual movement of the elevator as it deals with incoming requests. If the queue of requests is empty and the ElevatorManager has no destination, the Elevator stops and waits for a request. Once a request comes in, we move the elevator towards that request to service it. The Elevator waits on the Move condition until the handler signals it to move. Once the Elevator moves, it checks if it has reached the "atFloor" of the current request. If it has, the Elevator then beings to move toward the "toFloor" of the request. If the Elevator has reached the "toFloor", it waits for the next request. At each floor, the Elevator broadcasts to every thread waiting on that floor, and the thread determines whether to board or not. 

	-Tests: we spawn several requests to the elvator and print output accordingly.

	- to run our tests call: "./threads/nachos -P 5 -rs <someNum>"
	  we need random time slicing on


****SAMPLE OUTPUT****
// run using "./threads/nachos -P 5 -rs 10000"

No Requests. Elevator stopped at Floor: 0			// no requests elevator waits

Request(atFloor: 4, toFloor: 12) Waiting On Floor: 4 // new request

--NEW REQUEST-- Next Destination: 4 // elevator sees new request

ELEVATOR CURRENT FLOOR: 0. DESTINATION: 4 // begin to move toward new request
Request(atFloor: 3, toFloor: 1) Waiting On Floor: 3 // more requests wait
Request(atFloor: 6, toFloor: 8) Waiting On Floor: 6
Request(atFloor: 10, toFloor: 2) Waiting On Floor: 10
ELEVATOR CURRENT FLOOR: 1. DESTINATION: 4
ELEVATOR CURRENT FLOOR: 2. DESTINATION: 4
ELEVATOR CURRENT FLOOR: 3. DESTINATION: 4  // request at floor 3 does not get on
ELEVATOR CURRENT FLOOR: 4. DESTINATION: 12

Picked Up Request(atFloor: 4. toFloor: 12)  // picked up request, move to request dest.

ELEVATOR CURRENT FLOOR: 5. DESTINATION: 12

Picked Up Request(atFloor: 6. toFloor: 8)  // we can service this request on the way, request
										   // gets in

ELEVATOR CURRENT FLOOR: 6. DESTINATION: 12
ELEVATOR CURRENT FLOOR: 7. DESTINATION: 12

Dropped Off Request(atFloor: 6, toFloor: 8) on Floor: 8   // dropped off request

ELEVATOR CURRENT FLOOR: 8. DESTINATION: 12
ELEVATOR CURRENT FLOOR: 9. DESTINATION: 12
ELEVATOR CURRENT FLOOR: 10. DESTINATION: 12   // request at 10 does not get on
ELEVATOR CURRENT FLOOR: 11. DESTINATION: 12
ELEVATOR CURRENT FLOOR: 12. DESTINATION: 3

Dropped Off Request(atFloor: 4, toFloor: 12) on Floor: 12  // dropped off request
															// elevator empty so move to next
															// request on queue

ELEVATOR CURRENT FLOOR: 11. DESTINATION: 3
ELEVATOR CURRENT FLOOR: 10. DESTINATION: 3   // request at 10 does not get on
ELEVATOR CURRENT FLOOR: 9. DESTINATION: 3
ELEVATOR CURRENT FLOOR: 8. DESTINATION: 3
ELEVATOR CURRENT FLOOR: 7. DESTINATION: 3
ELEVATOR CURRENT FLOOR: 6. DESTINATION: 3
ELEVATOR CURRENT FLOOR: 5. DESTINATION: 3
ELEVATOR CURRENT FLOOR: 4. DESTINATION: 3
ELEVATOR CURRENT FLOOR: 3. DESTINATION: 1 

Picked Up Request(atFloor: 3. toFloor: 1)	// pick up request at 3

ELEVATOR CURRENT FLOOR: 2. DESTINATION: 1

Dropped Off Request(atFloor: 3, toFloor: 1) on Floor: 1  // drop off request at 1
														// move to next request at 10

ELEVATOR CURRENT FLOOR: 1. DESTINATION: 10
ELEVATOR CURRENT FLOOR: 2. DESTINATION: 10
ELEVATOR CURRENT FLOOR: 3. DESTINATION: 10
ELEVATOR CURRENT FLOOR: 4. DESTINATION: 10
ELEVATOR CURRENT FLOOR: 5. DESTINATION: 10
ELEVATOR CURRENT FLOOR: 6. DESTINATION: 10
ELEVATOR CURRENT FLOOR: 7. DESTINATION: 10
ELEVATOR CURRENT FLOOR: 8. DESTINATION: 10
ELEVATOR CURRENT FLOOR: 9. DESTINATION: 10
ELEVATOR CURRENT FLOOR: 10. DESTINATION: 2

Picked Up Request(atFloor: 10. toFloor: 2)  // pick up at 10

ELEVATOR CURRENT FLOOR: 9. DESTINATION: 2
ELEVATOR CURRENT FLOOR: 8. DESTINATION: 2
ELEVATOR CURRENT FLOOR: 7. DESTINATION: 2
ELEVATOR CURRENT FLOOR: 6. DESTINATION: 2
ELEVATOR CURRENT FLOOR: 5. DESTINATION: 2
ELEVATOR CURRENT FLOOR: 4. DESTINATION: 2
ELEVATOR CURRENT FLOOR: 3. DESTINATION: 2

No Requests. Elevator stopped at Floor: 2


Dropped Off Request(atFloor: 10, toFloor: 2) on Floor: 2  // drop off at 2
