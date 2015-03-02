//	elevator.h
//	 Defines the interface for an ElevatorManager in order
//	 to solve the elevator problem in the Threads project.
//
//	 Student and Faculty Threads call ArrivingGoingFromTo,
//	 signaling the Elevator to move either up or down and
//	 service the requests on each floor in the same direction




#ifdef CHANGED
#ifndef ELEVATOR_H
#define ELEVATOR_H

#include "synch.h"
#include "system.h"
#include <string>
#include <iostream>

#define NO_DESTINATION -1		// signifies the Elevator is not moving towards a destination

enum ELEVATOR_DIRECTION {STOPPED, GOING_UP, GOING_DOWN}; // defines states of the Elevator

// timer interrupt handler for the ElevatorManager that handles when
// the Elevator moves a floor up or down;
void MovedFloor(int );

class ElevatorRequest {
	public:
		int atFloor, toFloor;	// floor request is at and floor it wishes to go to
		ELEVATOR_DIRECTION direction;	// direction the request is in

		ElevatorRequest(int atF, int toF);	// constructs a new ElevatorRequest
};



class ElevatorManager {
	public:

		// construct a new ElevatorManager that controls an Elevator
		// that services floors in range [0, n)
		ElevatorManager(const char *debugName, int n); 

		void ArrivingGoingFromTo(int atFloor, int toFloor);	// Students call this function to signal the Elevator to move
		void UpdateQueue(ElevatorRequest *finishedRequest);	//Updating the queue after someone has been dropped
		const char* getName() { return name;}			// debugging assist

		const char *name;
		int numFloors;		// total number of floors
		int curFloor;		// keeps track of the current floor the Elevator is on
		int destination;		// floor number the elevator is moving towards
		ELEVATOR_DIRECTION direction;		// keeps track of the state of the Elevator
		
		List *reqQueue;	// List used to signal that there are requests that the elevator must service
		

		Timer *timer;	// timer used to govern movement of Elevator
		
		Condition **floors;	// array of Conditions in which Students will wait on for the elevator to pick them up/drop off
		Condition *request;	// an Elevator waits for a request. Students signal that there are requests
		Condition *move;	// An Elevator waits for a move. the Handler signals that a move occurred

		Lock *lock;		// provides protection to the elevator

	private:
		bool IsInRange(int atFloor, int toFloor);	// returns true if (atFloor, toFloor) in range(curFloor, destination)
};

#endif // ELEVATOR_H
#endif // CHANGED