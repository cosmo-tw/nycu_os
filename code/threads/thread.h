// thread.h 
//	Data structures for managing threads.  A thread represents
//	sequential execution of code within a program.
//	So the state of a thread includes the program counter,
//	the processor registers, and the execution stack.
//	
// 	Note that because we allocate a fixed size stack for each
//	thread, it is possible to overflow the stack -- for instance,
//	by recursing to too deep a level.  The most common reason
//	for this occuring is allocating large data structures
//	on the stack.  For instance, this will cause problems:
//
//		void foo() { int buf[1000]; ...}
//
//	Instead, you should allocate all data structures dynamically:
//
//		void foo() { int *buf = new int[1000]; ...}
//
//
// 	Bad things happen if you overflow the stack, and in the worst 
//	case, the problem may not be caught explicitly.  Instead,
//	the only symptom may be bizarre segmentation faults.  (Of course,
//	other problems can cause seg faults, so that isn't a sure sign
//	that your thread stacks are too small.)
//	
//	One thing to try if you find yourself with seg faults is to
//	increase the size of thread stack -- ThreadStackSize.
//
//  	In this interface, forking a thread takes two steps.
//	We must first allocate a data structure for it: "t = new Thread".
//	Only then can we do the fork: "t->fork(f, arg)".
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef THREAD_H
#define THREAD_H

#include "copyright.h"
#include "utility.h"
#include "sysdep.h"
#include "machine.h"
#include "addrspace.h"

// CPU register state to be saved on context switch.  
// The x86 needs to save only a few registers, 
// SPARC and MIPS needs to save 10 registers, 
// the Snake needs 18,
// and the RS6000 needs to save 75 (!)
// For simplicity, I just take the maximum over all architectures.

#define MachineStateSize 75 


// Size of the thread's private execution stack.
// WATCH OUT IF THIS ISN'T BIG ENOUGH!!!!!
const int StackSize = (8 * 1024);	// in words


// Thread state
enum ThreadStatus { JUST_CREATED, RUNNING, READY, BLOCKED, ZOMBIE };


// The following class defines a "thread control block" -- which
// represents a single thread of execution.
//
//  Every thread has:
//     an execution stack for activation records ("stackTop" and "stack")
//     space to save CPU registers while not running ("machineState")
//     a "status" (running/ready/blocked)
//    
//  Some threads also belong to a user address space; threads
//  that only run in the kernel have a NULL address space.

class Thread {
  private:
    // NOTE: DO NOT CHANGE the order of these first two members.
    // THEY MUST be in this position for SWITCH to work.
    int *stackTop;			 // the current stack pointer
    void *machineState[MachineStateSize];  // all registers except for stackTop

  public:
    Thread(char* debugName, int threadID);		// initialize a Thread 
    ~Thread(); 				// deallocate a Thread
					            // NOTE -- thread being deleted
					            // must not be running when delete 
					            // is called

    // basic thread operations

    void Fork(VoidFunctionPtr func, void *arg); // Make thread run (*func)(arg)			
    void Yield();  		                          // Relinquish the CPU if any other thread is runnable
    void Sleep(bool finishing);                 // Put the thread to sleep and relinquish the processor
    void Begin();		                            // Startup code for the thread	
    void Finish();  		                        // The thread is done executing
    void CheckOverflow();   	                  // Check if thread stack has overflowed
    void setStatus(ThreadStatus st) { status = st; }
    void SelfTest();		                        // test whether thread impl is working

    ThreadStatus getStatus() { return (status); }
	  char* getName() { return (name); }
    void Print() { cout << name; }
	  int getID() { return (ID); }
    

    /* HW4 new add ===================================== */ 
    void setPriority(int p) { priority = p; }
    int getPriority() { return priority; }
    int getLevel() 
    { return 3 - priority / 50;}
    void setBurstTime(double t) { burstTime = t; }
    double getBurstTime() { return burstTime; }
    void setPredictTime(double t) { predictTime = t; }
    double getPredictTime() { return predictTime; }
    void setRemainingTime(double t) { remainingTime = t; }
    double getRemainingTime() { return remainingTime; }
    void setRunningTime(double t) { runningTime = t; }
    double getRunningTime() { return runningTime; }
    void setWaitingTime(double t) { waitingTime = t; }
    double getWaitingTime() { return waitingTime; }
    void setTotalWaitingTime(double t) { totalWaitingTime = t; }
    double getTotalWaitingTime() { return totalWaitingTime; }
    /*===================================================*/

  private:
    // some of the private data for this class is listed above
    int *stack; 	 	// Bottom of the stack 
				            // NULL if this is the main thread
				            // (If NULL, don't deallocate stack)
    ThreadStatus status;	// ready, running or blocked
    char* name;
	  int   ID;

    // Allocate a stack for thread.
		// Used internally by Fork()
    void StackAllocate(VoidFunctionPtr func, void *arg);

    /* HW4 new add ===================================== */
    int priority;            // scheduling priority (0~149)
    double burstTime;        // accumulating job execution time
    double predictTime;      // the predict time for next CPU burst
    double remainingTime;    // remaining time
    double runningTime;      // the time of entering the Running state
    double waitingTime;      // the time of start waiting
    double totalWaitingTime; // the total wating time
    /*===================================================*/
    		

// A thread running a user program actually has *two* sets of CPU registers -- 
// one for its state while executing user code, one for its state 
// while executing kernel code.

    int userRegisters[NumTotalRegs];	// user-level CPU register state

  public:
    void SaveUserState();		// save user-level register state
    void RestoreUserState();		// restore user-level register state

    AddrSpace *space;			// User code this thread is running.
};

// external function, dummy routine whose sole job is to call Thread::Print
extern void ThreadPrint(Thread *thread);	 

// Magical machine-dependent routines, defined in switch.s

extern "C" {
// First frame on thread execution stack; 
//   	call ThreadBegin
//	call "func"
//	(when func returns, if ever) call ThreadFinish()
void ThreadRoot();

// Stop running oldThread and start running newThread
void SWITCH(Thread *oldThread, Thread *newThread);
}

#endif // THREAD_H
