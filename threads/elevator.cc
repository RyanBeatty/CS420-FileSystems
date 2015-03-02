#ifdef CHANGED
//	elevator.cc
//	implements logic for the ElevatorManager which manages requests to the Elevator
//
//	Elevator Algorithm:
//	The Elevator should then service requests, prioritizing requests
//	made first and picking up students who can be serviced on the
//	way to the next request.
//
//	Request Algorithm:
//	 A request comes into the ElevatorManager and checks if it needs to be put on the request Queue.	
//	A request must go onto the queue if 1. the elevator is not going in the same direction as the my direction.
//	or 2. (atFloor, toFloor) of the request is not within range(curFloor, destination) of the Elevator.
//
//	 There is a Condition array with one Condition per floor. Each reqeust waits on its atFloor Condition
//	in the Condition array until the Elevator picks it up. It then waits on its toFloor condition in the Condition
//	array until the Elevator drops it off. 

#include "elevator.h"

//----------------------------------------------------------------------
// ElevatorManager::MovedFloor
// 	handler function for the timer. Checks current status of the Elevator
//	and moves in the direction the elevator is moving
//----------------------------------------------------------------------
void
MovedFloor(int m) {
	ElevatorManager *manager = (ElevatorManager *) m;

	Lock dummyLock("dummy");			// need dummy lock that no thread owns, else Signal will throw error
	dummyLock.Acquire();
	manager->move->Signal(&dummyLock);	// signal that the Elevator should move
}

//----------------------------------------------------------------------
// ElevatorRequest::ElevatorRequest
// 	Initialize an ElevatorRequest that stores info about a request
//	to the Elevator.
//
//	"atFloor" - the floor the request is at
//	"toFloor" - the floor the request wants to go to
//	NOTE: atFloor != toFloor
//----------------------------------------------------------------------
ElevatorRequest::ElevatorRequest(int atF, int toF) {
	ASSERT(atF != toF);				// requests should move at least one floor
	atFloor = atF;
	toFloor = toF;
	direction = toFloor - atFloor > 0 ? GOING_UP : GOING_DOWN;	// calculate direction the request is in
}

//----------------------------------------------------------------------
// ElevatorManager::ElevatorManager
// 	Initialize an ElevatorManager that will control an Elevator and 
//	manage requests from passengers
//
//	"n" is the range of floors for the elevator [0, n)
//----------------------------------------------------------------------
ElevatorManager::ElevatorManager(const char *debugName, int n) {
	numFloors = n;
	curFloor = 0;	// start Elevator out on floor 0
	
	direction = STOPPED;				// the Elevator is stopped at the beginning
	destination = NO_DESTINATION; 	// the Elevator has no destination at the beginning

	reqQueue = new(std::nothrow) List;							// initialize queue of requests
	timer = new(std::nothrow) Timer(MovedFloor, (int) this, false);	// intialize timer and pass handler funciton in
	floors = new(std::nothrow) Condition*[numFloors];	// make an array of Condition objects, one for each floor
	request = new(std::nothrow) Condition(debugName);	// Elevator waits for a request. Students signal requests
	move = new(std::nothrow) Condition(debugName);		// Elevator waits for moves. handler signales when moved
	lock = new(std::nothrow) Lock(debugName);		// provide mutex

	for(int i = 0; i < numFloors; ++i)			// initialize each floor to be a condition variable
		floors[i] = new(std::nothrow) Condition("1");
}

//----------------------------------------------------------------------
// ElevatorManager::ArrivingGoingFromTo
// 	 The request made by an arriving person from a floor going to another floor
// 	Add the person to the queue if they cannot be dropped off on the way then
//	wait at atFloor for the Elevator to pick up. Once in the elevators, the request
//	then waits on its toFloor for the Elevator to drop it off.
//----------------------------------------------------------------------
void 
ElevatorManager::ArrivingGoingFromTo(int atFloor, int toFloor) {
	ElevatorRequest *myRequest = new(std::nothrow) ElevatorRequest(atFloor, toFloor);	// create new request object

	lock->Acquire();
	if(direction != STOPPED) {		// elevator is busy
		if(direction != myRequest->direction || !IsInRange(atFloor, toFloor)) {		// we cant service this request on the way to the current request
			reqQueue->Append((void *) myRequest);		// so add request to the end of the request queue
		}
	}
	else{
		reqQueue->Append((void *) myRequest);	 // else no requests so add my request to queue
	}

	printf("Request(atFloor: %d, toFloor: %d) Waiting On Floor: %d\n", atFloor, toFloor, atFloor);
	fflush(stdout);

	// keep waiting at atFloor unless Elevator is moving in the same
	// direction as my request, and my request is in the range of the current
	// request the Elevator is servicing (i.e. [toFloor, atFloor] in range[curFloor, destination])
	do {
		request->Signal(lock);
		floors[atFloor]->Wait(lock);
	} while(direction != myRequest->direction || !IsInRange(atFloor,toFloor));

	printf("\nPicked Up Request(atFloor: %d. toFloor: %d)\n\n", curFloor, toFloor);
	fflush(stdout);

	
	floors[toFloor]->Wait(lock);		// wait on toFloor to get off elevator

	UpdateQueue(myRequest);		// remove myRequest from request queue, so there are no superflous requests  

	printf("\nDropped Off Request(atFloor: %d, toFloor: %d) on Floor: %d\n\n", atFloor, toFloor, curFloor);
	fflush(stdout);

	lock->Release();
}

//----------------------------------------------------------------------
// ElevatorManager::UpdateQueue
// 
// Update the queue in case there are any superflous requests after someone has been dropped off
//----------------------------------------------------------------------
void ElevatorManager::UpdateQueue(ElevatorRequest *finishedRequest){
	List *newReqQueue = new(std::nothrow) List();
	ElevatorRequest *tmpReq = (ElevatorRequest *) reqQueue->Remove();
	
	while(tmpReq != NULL){				// iterate over request queue, and remove finishedRequest
		if(tmpReq->atFloor != finishedRequest->atFloor && tmpReq->toFloor != finishedRequest->toFloor){
			newReqQueue->Append((void *) tmpReq);
		}
		tmpReq = (ElevatorRequest *) reqQueue->Remove();
	}
	reqQueue = newReqQueue;
}

//----------------------------------------------------------------------
// ElevatorManager::IsInRange
// 
// returns true if (atFloor, toFloor) in range(curFloor, destination)
//----------------------------------------------------------------------
bool
ElevatorManager::IsInRange(int atFloor, int toFloor) {
	switch(direction) {
		case GOING_UP:
			if((atFloor >= curFloor && atFloor <= destination) && (toFloor >= curFloor && toFloor <= destination))
				return true;
			break;
		case GOING_DOWN:
			if((atFloor <= curFloor && atFloor >= destination) && (toFloor <= curFloor && toFloor >= destination))
				return true;
			break;
		case STOPPED:
			ASSERT(false);
			return false;
			break;
	}
	return false;
}

#endif // CHANGED