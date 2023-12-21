// scheduler.cc 
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "debug.h"
#include "scheduler.h"
#include "main.h"

/* HW3 new add - but no use in HW4 */
// int pqSchedulingCompare (Thread *x, Thread *y)
// {
//     if (x->getpriority() < y->getpriority()) { return 1; } // x has higher priority
//     else if (x->getpriority() > y->getpriority()) { return -1; } // y has higher priority
//     else { return 0; } // priorities are equal
// }

/* HW4 new add======================================================================= */
int compareRemainingTime(Thread* x, Thread* y)
{
if (x->getRemainingTime() == y->getRemainingTime()) return 0;
return (x->getRemainingTime() > y->getRemainingTime()) ? 1 : -1;
}
int comparePriority(Thread* x, Thread* y)
{
if (x->getPriority() == y->getPriority()) return 0;
return (x->getPriority() < y->getPriority()) ? 1 : -1;
}
/*==================================================================================*/

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------
Scheduler::Scheduler()
{ 
    // readyList = new List<Thread *>; 
    // pqList = new SortedList<Thread*>(pqSchedulingCompare);
    L1ReadyList = new SortedList<Thread *>(compareRemainingTime);
    L2ReadyList = new SortedList<Thread *>(comparePriority);
    L3ReadyList = new List<Thread *>;

    toBeDestroyed = NULL;
} 

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler()
{ 
    // delete readyList; 
    // delete pqList;

    /* HW4 new add */
    delete L1ReadyList;
    delete L2ReadyList;
    delete L3ReadyList;
} 

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void
Scheduler::ReadyToRun (Thread* thread)
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    DEBUG(dbgThread, "Putting thread on ready list: " << thread->getName());
	//cout << "Putting thread on ready list: " << thread->getName() << endl ;

    /* HW4 new add ========================================================== */
    thread->setWaitingTime(kernel->stats->totalTicks);

    cout << "thread " << thread->getName() << "'s priority is" << thread->getPriority() << "\n";

    int level = thread->getLevel();
    if (level == 1)
    {
        L1ReadyList->Insert(thread);
    }
    else if (level == 2)
    {
        L2ReadyList->Insert(thread);
    }
    else
    {
        L3ReadyList->Append(thread);
    }
    thread->setStatus(READY);
    DEBUG(dbtwo, "[A] Tick [" << kernel->stats->totalTicks << "]:Thread [" << thread->getID() << "] is inserted into queue L[" << level << "]");
    /*========================================================================*/
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun ()
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    int level;
    Thread *nextThread = NULL;

    if (!L1ReadyList->IsEmpty())
    {
        nextThread = L1ReadyList->RemoveFront();
        level = 1;
    }
    else if (!L2ReadyList->IsEmpty())
    {
        nextThread = L2ReadyList->RemoveFront();
        level = 2;
    }
    else if (!L3ReadyList->IsEmpty())
    {
        nextThread = L3ReadyList->RemoveFront();
        level = 3;
    }
    else
    {
        return NULL;
    }
    DEBUG(dbtwo, "[B] Tick [" << kernel->stats->totalTicks << "]:Thread [" << nextThread->getID() << "] is removed from queue L[" << level << "]");
    return nextThread;
}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable kernel->currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//	"finishing" is set if the current thread is to be deleted
//		once we're no longer running on its stack
//		(when the next thread starts running)
//----------------------------------------------------------------------

void
Scheduler::Run (Thread *nextThread, bool finishing)
{
    Thread *oldThread = kernel->currentThread;
    
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    if (finishing) 
    {	// mark that we need to delete current thread
        ASSERT(toBeDestroyed == NULL);
	    toBeDestroyed = oldThread;
    }
    
    if (oldThread->space != NULL) 
    {	                                // if this thread is a user program,
        oldThread->SaveUserState(); 	// save the user's CPU registers
	    oldThread->space->SaveState();
    }
    
    oldThread->CheckOverflow();		    // check if the old thread had an undetected stack overflow

    kernel->currentThread = nextThread;  // switch to the next thread
    nextThread->setStatus(RUNNING);      // nextThread is now running
    
    DEBUG(dbgThread, "Switching from : " << oldThread->getName() << " to: " << nextThread->getName());
    
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".

    nextThread->setRunningTime(kernel->stats->totalTicks); // update running time
    nextThread->setTotalWaitingTime(0);                    // reset total waiting time
    SWITCH(oldThread, nextThread);

    // we're back, running oldThread
      
    // interrupts are off when we return from switch!
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    DEBUG(dbgThread, "Now in thread: " << oldThread->getName());

    CheckToBeDestroyed();		// check if thread we were running
					// before this one has finished
					// and needs to be cleaned up
    
    if (oldThread->space != NULL) {	    // if there is an address space
        oldThread->RestoreUserState();     // to restore, do it.
	oldThread->space->RestoreState();
    }

}

//----------------------------------------------------------------------
// Scheduler::CheckToBeDestroyed
// 	If the old thread gave up the processor because it was finishing,
// 	we need to delete its carcass.  Note we cannot delete the thread
// 	before now (for example, in Thread::Finish()), because up to this
// 	point, we were still running on the old thread's stack!
//----------------------------------------------------------------------

void
Scheduler::CheckToBeDestroyed()
{
    if (toBeDestroyed != NULL) {
        delete toBeDestroyed;
	toBeDestroyed = NULL;
    }
}
 
//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------
void
Scheduler::Print()
{
    cout << "Ready list contents:\n";
    readyList->Apply(ThreadPrint);
}
