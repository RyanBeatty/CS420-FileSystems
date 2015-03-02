#ifdef CHANGED
// elevatortest.cc
//
//	Test suite for the Elevator problem in Nachos1.
//	Spawn several Student Threads that make different
//	requests on the Elevator. The Elevator should then service requests, prioritizing requests
//	made first and picking up students who can be serviced on the
//	way to the next request.

#include "elevator.h"


static ElevatorManager *manager;	// manages requests to the elevator


//----------------------------------------------------------------------
//	Student
//	Struct used to hold request info. A student struct is passed to
//	a Student thread and makes a request on the elevator
//----------------------------------------------------------------------
struct Student{
	int atFloor;
	int toFloor;
};

//----------------------------------------------------------------------
//	GetStudent
//	Returns a Student struct with at/to floor set appropriately
//----------------------------------------------------------------------
struct
Student *GetStudent(int at, int to){
	struct Student *newPerson = (struct Student *) malloc(sizeof(struct Student));
	newPerson->atFloor =  at;
	newPerson->toFloor = to;
	return newPerson;
}

//----------------------------------------------------------------------
//	Request
//	A request to the elevator, takes in an int parameter which is a 
//	Student struct cast to an int, and makes the appropriate request
//----------------------------------------------------------------------
void
Request(int which) {
	struct Student *requester = (struct Student *)  which;
	manager->ArrivingGoingFromTo(requester->atFloor, requester->toFloor);	// make request on elevator
}

//----------------------------------------------------------------------
//	SetNextRequest
//	Pops the next request for the Elevator off of the request Queue
//	and sets the elevator to move towards that request
//----------------------------------------------------------------------
ElevatorRequest * 
SetNextRequest() {
	ElevatorRequest *curRequest = (ElevatorRequest *) manager->reqQueue->Remove();	// pop off next request and go there
	manager->destination = curRequest->atFloor;

	int nextDirection = manager->destination - manager->curFloor;	// find direction to next request
	
	if(nextDirection == STOPPED)			// at the same floor as next request, so stay stopped
		manager->direction = STOPPED;
	else if(nextDirection > 0)				// below next request, go up
		manager->direction = GOING_UP;
	else									// above next request, go down
		manager->direction = GOING_DOWN;

	return curRequest;
}


//----------------------------------------------------------------------
//	Elevator
//	Thread which denotes the Elevtor. The elevator will wait for requests
//	from other Student Threads and then move to service them.
//----------------------------------------------------------------------
void
Elevator(int which) {

	ElevatorRequest *curRequest;
	while(true) {
		manager->lock->Acquire();

		if(manager->reqQueue->IsEmpty() && manager->destination == NO_DESTINATION) {	// no requests, no destination. elevator should stop
			printf("\nNo Requests. Elevator stopped at Floor: %d\n\n", manager->curFloor);
			fflush(stdout);

			manager->direction = STOPPED;
			manager->request->Wait(manager->lock);			// wait for a request to come in

			curRequest = SetNextRequest();						// get next request and move to service it

			printf("\n--NEW REQUEST-- Next Destination: %d\n\n", manager->destination);
			fflush(stdout);
		} 
		else if(manager->destination == NO_DESTINATION) { 	// there are still requests, pop next request from queue
			curRequest = SetNextRequest();			// get next request and move to service it

			// printf("\n--NEW REQUEST-- Next Destination: %d\n\n", manager->destination);
			// fflush(stdout);
		}

		//----------AT THIS POINT WE HAVE A REQUEST THAT WE ARE SEVICING-----------//

		printf("ELEVATOR CURRENT FLOOR: %d. DESTINATION: %d\n", manager->curFloor, manager->destination);
		fflush(stdout);

		manager->move->Wait(manager->lock);			// wait to move a floor

		if(manager->direction == GOING_UP)			// move floor up if going up
			manager->curFloor++;
		else										// move floor down if going down
			manager->curFloor--;

		ASSERT(manager->curFloor >= 0);					// should be within floor range still [0, numFloors)
		ASSERT(manager->curFloor < manager->numFloors);

		if(manager->curFloor == curRequest->atFloor) {		// if elevator at request start floor
			manager->destination = curRequest->toFloor;	// switch current destination to be request destination
			manager->direction = curRequest->direction;	// switch current direction to be towards request destination
		}
		else if(manager->curFloor == manager->destination) {	// if elevator at request destination
			ASSERT(manager->destination == curRequest->toFloor);		// curFloor should be the toFloor of the current request
			manager->destination = NO_DESTINATION;		// stop elevator for now, check later for more requests
		}

		manager->floors[manager->curFloor]->Broadcast(manager->lock);	// broadcast to everyone on floor to either get on/off or queue up
		manager->lock->Release();

	}
}

//----------------------------------------------------------------------
//	ElevatorTest
//	Initialize the elevator and fork the appropriate threads
//----------------------------------------------------------------------
void
ElevatorTest(int numFloors) {
	manager = new(std::nothrow) ElevatorManager("manager", numFloors);

	struct Student *firstPerson = GetStudent(4,12);
	struct Student *secondPerson = GetStudent(3,1);
	struct Student *thirdPerson = GetStudent(6,8);
	struct Student *fourthPerson = GetStudent(10,2);

	Thread *requester = new(std::nothrow) Thread("requester");
	requester->Fork(Request, (int) firstPerson);

	Thread *requester2 = new(std::nothrow) Thread("requester");
	requester2->Fork(Request, (int) secondPerson);

	Thread *req3 = new(std::nothrow) Thread("requester");
	req3->Fork(Request, (int) thirdPerson);

	Thread *req4 = new(std::nothrow) Thread("requester");
	req4->Fork(Request, (int) fourthPerson);

	Elevator(1);
}

#endif // CHANGED