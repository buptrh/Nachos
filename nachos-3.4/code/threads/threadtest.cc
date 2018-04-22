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
//#define CHANGED            // When defined student code will run
//#define HW1_SEMAPHORES    // When defined sync code will be ran
//#define HW1_LOCKS         // Use Locks to synch the code
//#define HW1_ELEVATOR 
#if defined(CHANGED)
#include "utility.h"
#include "synch.h"
#endif

// testnum is set in main.cc
int testnum = 1;


/* Allow selection of original or student code at compile time */
#if defined(CHANGED)
int live_threads = 0;

/* Allow selection of semaphore, lock or elevator test at compile time */
#if defined(HW1_ELEVATOR)

/* Definitions */
#define MAX_CAPACITY 5
#define MIN_FLOOR    1
#define TICK_DELAY   50

/* Global Vars and objs */
int cur_floor  = 1; // Set initially to 0 so that elevator will visit floor 1 
int occupants  = 0;
int passid     = 0;
int target     = 1;
int lvl_count  = 0;
int pass_count = 0;
bool up        = true;

/* Containts the attributes of a person */
struct Person {
  int id;
  int from;
  int to;
  bool going_up;
};

/* Mutual exclusion for the elevator access and level barrier */
Lock * e_mutex   = new Lock("Lock Elevator");
Lock * lvl_mutex = new Lock("Lock Level");

/* Conditions in order to allow waiting and signaling */
Condition * elevator  = new Condition ("Cond Eleveator");
Condition * passenger = new Condition ("Cond Passenger F");
Condition * level     = new Condition ("Cond level");

#elif defined(HW1_SEMAPHORES)
Semaphore s_mutex("Semaphore ThreadTest", 1); 
#elif defined(HW1_LOCKS)
Lock l_mutex ("Lock ThreadTest");
#endif

/*Global variable which will be hit by all threads in SimpleThread */
int SharedVariable;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration. use Preprocessor tags to enable synchronization
//      through different methods. 
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------
void SimpleThread(int which) {

  DEBUG('t', "%s is entering pre-provided SimpleThread", currentThread -> getName());
  /* 
     Decrement the semaphore when accessing global variable and
     increment once access is compelete to allow other threads 
     to execute. 
  */
  int num, val;
  // Begin for-loop
  for(num = 0; num < 5; num++) {
   
#if defined(HW1_SEMAPHORES)
    s_mutex.P();
    DEBUG('t', "Semaphore decrement visisted by %s", currentThread -> getName());
#elif defined(HW1_LOCKS)
    l_mutex.Acquire();
    DEBUG('t', "Lock acquire visisted by %s", currentThread -> getName());
#endif

    val = SharedVariable;
    printf("*** thread %d sees value %d\n", which, val);
    currentThread->Yield();
    SharedVariable = val+1;

#if defined(HW1_SEMAPHORES)
    s_mutex.V();
    DEBUG('t', "Semaphore increment visisted by %s", currentThread -> getName());
#elif defined(HW1_LOCKS)
    l_mutex.Release();
    DEBUG('t', "Lock release visisted by %s", currentThread -> getName());
#endif
    currentThread->Yield(); 
  } // End for-loop
  
  /* Barrier required to synchronize final value shown */
  live_threads--;
  DEBUG('t', "live_threads=%d\n", live_threads);
  while (live_threads > 0 )
    currentThread->Yield();

  /* Display final values */
  val = SharedVariable;  
  printf("Thread %d sees final value %d\n", which, val);  
}  
#else

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
#endif

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------
void
ThreadTest1()
{
  DEBUG('t', "Entering ThreadTest1");
  
  Thread *t = new Thread("forked thread");
  
  t->Fork(SimpleThread, 1);
  SimpleThread(0);
}


//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------
void
ThreadTest()
{
  switch (testnum) {
  case 1:
    ThreadTest1();
    break;
  default:
    printf("No test specified.\n");
    break;
    }
}


#if defined(CHANGED)

//----------------------------------------------------------------------
// ThreadTest 
// 	Generate a set of threads to run the SimpleThread function. 
//
//      Where n is an integer specifing yhe amount of threads to be 
//      forked
//----------------------------------------------------------------------
void
ThreadTest(int n)
{
  DEBUG('t', "Entering Student defined ThreadTest.");
  live_threads = n + 1;
  DEBUG('t', "live_threads set to %d.", live_threads);

  if ( n > 0) {
    if( n > 1 ) {
      Thread *t;
      int i;
      char buf[10];
      
      /* Allocate and Fork new threads. Execute SimpleThread */
      for (i = 1 ; i <= n ; i++) {
	sprintf(buf, "Thread %d", i);
        t = new Thread(buf);
        t -> Fork(SimpleThread, i);
      }
      t = NULL;     // Erase reference 
      delete t;     // Delete pointer
    }
    SimpleThread(0); // Run the current thread

  } else
    DEBUG('t', "Number of threads set to 0. Exiting.");
}

#if defined(HW1_ELEVATOR)
//----------------------------------------------------------------------
//  AtSameFloor
//      Check if the passenger's floor and the cur_floor global variable
//      are the same.
//
//      Where "floor" is the floor to be compared
//
//      Pre-condition: This function must be used inside of the critical
//                     section. Access must be synchronized.
//----------------------------------------------------------------------
bool
AtSameFloor(int floor)
{
  return (floor == cur_floor) ? true : false;
}

//----------------------------------------------------------------------
//  DelayE
//      Delays the systems by the specified number of ticks. Each
//      integer addition is a single tick in the MIPS simulator.
//
//      Where "delay_ticks" is considered to be the amount of ticks
//----------------------------------------------------------------------
void
DelayE(int delay_ticks)
{
  int ticks; 
  for (ticks = 0 ; ticks < delay_ticks ; ticks++)
    ;
}

//----------------------------------------------------------------------
//  Elevator
// 	Models an elevator which carries a set amount of passengers
//      from their origins and to their destinations. The elevator
//      begins at the first floor and begins going up the building
//      collecting passengers based on their requests.
//
//      Where "numFloors" is considered to be the maximum amount of
//      floors
//----------------------------------------------------------------------
void
Elevator(int numFloors)
{
  /* 
     Don't allow a building with no floors or that is 
     outside the max range
  */
  ASSERT (numFloors > 0 && numFloors < 200);
  printf("Elevator arrives on floor %d.\n", cur_floor);

  /* Execute the elevator infinitely */
  while(1) {
    e_mutex -> Acquire();

    /* Reset barrier to 0 */
    lvl_count = 0;

    /* Wait or passengers */
    while (pass_count == 0) {
      DEBUG('t',  "--Entering Wait on NO Passenegers\n");
      elevator -> Wait(e_mutex);
      DEBUG('t',  "--Exiting Wait on NO Passenegers\n");
    }    
    passenger -> Broadcast(e_mutex);
    e_mutex -> Release();
    
    lvl_mutex -> Acquire();
    while (lvl_count < pass_count) 
      level -> Wait(lvl_mutex);

    /* 
       Highest floor that a passenger specified has been reached. 
       Change directions and cease this iteration in order to pick
       up the next passenger.
    */
    lvl_mutex -> Release();
    if (cur_floor == target) {
      up = !up;
      continue;
    }

    e_mutex -> Acquire();
    /* Cease this iteration if no passengers */
    if (pass_count == 0) {
      continue;
    }

    /* Add specified tick delay between floors */
    DelayE(TICK_DELAY);

    /* Move through a building in order to reach the desired floor */
    if (cur_floor != target) {
   
      DEBUG ('t', "--E| cur_floor: %d, target, %d, numfloors: %d\n",
	     cur_floor, target, numFloors);
      /* 
	 Increase level is the target is above us and if the direction is
         up and the current floor is below the top floor. Otherwise do 
	 the opposite. 
      */
      if( (up && cur_floor < numFloors && cur_floor < target)) {
	cur_floor++;
	printf("Elevator arrives on floor %d.\n", cur_floor);    
      } else if ( (!up && cur_floor > MIN_FLOOR && cur_floor > target) ){
	cur_floor--;
	printf("Elevator arrives on floor %d.\n", cur_floor);    
      }
    }
    e_mutex -> Release();
  }
}

//----------------------------------------------------------------------
//  ArrivingGoingFromTo
// 	Generates a passenger which goes from one floor to another in a
//      using a thread that is conceptually an elevator.
//
//      atFloor is the passengers origination
//      toFloor is the passengers destination
//----------------------------------------------------------------------
void
ArrivingGoingFromTo(int atFloor, int toFloor)
{
  Person P; 

  /* Populate the structure */
  e_mutex -> Acquire();

  /* Alert the elevator that a passenger is in the building */
  if (pass_count == 0) 
    elevator->Signal(e_mutex);
 
  pass_count++;
  lvl_count++;
  
  P.id   = passid + 1;
  passid++;
  e_mutex -> Release();
  
  P.from = atFloor;
  P.to   = toFloor;
  P.going_up = (P.from < P.to) ? true : false;
  printf("Person %d wants to go to floor %d from floor %d.\n"
	 , P.id, P.to, P.from);

  /* Begin execution (call the elevator) */
  e_mutex -> Acquire();

  /* Wait if a valid boarding cannot be made by the passenger */
  while (occupants >= MAX_CAPACITY || !AtSameFloor(P.from)
	 || (P.going_up != up && P.from != P.to )) {
    DEBUG('t', "--P| Person %d waiting in initial while-loop! (TID): %s\n"
	  , P.id, currentThread -> getName());
    DEBUG('t', "--P| lvl_count: %d, pass_count: %d.", lvl_count, pass_count);
    passenger -> Wait(e_mutex);

    /* Reassign the elevator's target based on requests */
    if (up)
      target = max(P.from, target);
    else
      target = min(P.from, target);
    
    lvl_mutex -> Acquire();
    lvl_count++;
    level -> Signal(lvl_mutex);
    lvl_mutex -> Release();
  }

  occupants++;
  printf("Person %d got into the elevator.\n", P.id);

  
  while (!AtSameFloor(P.to)) {
    DEBUG('t', "--P| Person %d waiting inside elevator! (TID): %s\n"
	  , P.id, currentThread -> getName());
    DEBUG('t', "--P| lvl_count: %d, pass_count: %d.", lvl_count, pass_count);
    passenger -> Wait(e_mutex);

    /* If there exists only 1 occupant ensure direction matches */
    if (occupants == 1 && P.going_up != up)
      up = !up;

    /* Reassign the elevator's target based on the occupants */
    if (up) 
      target = max(P.to, target);
    else 
      target = min(P.to, target);

    lvl_mutex -> Acquire();
    lvl_count++;
    level -> Signal(lvl_mutex);
    lvl_mutex -> Release();
  }
  
  occupants--;
  printf("Person %d got out of the elevator.\n", P.id);

  lvl_mutex -> Acquire();
  pass_count--;
  lvl_count--;
  level -> Signal(lvl_mutex);
  lvl_mutex -> Release();
  
  e_mutex -> Release();

  /* 
     This thread has concluded set threadToBeDestoryed so that the thread
     can be destoryed 
  */
  currentThread -> Finish();
}

//----------------------------------------------------------------------
//  ArrivingGoingHelper
// 	Meant to be used in main when running a passenger thread. Allows
//      the passage of multiple paramaters to the ArrivingGoingFromTo
//      function.
//
//      Where "param" is used to pass a structure while adhering to the
//      functionality specified in Fork from the Thread class
//----------------------------------------------------------------------
void ArrivingGoingHelper(int param) {
  argE * a = (argE*) param;
  ArrivingGoingFromTo(a->from, a->to);
  delete a;
}

#endif
#else



#endif


