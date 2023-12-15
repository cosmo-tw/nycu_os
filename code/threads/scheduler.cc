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

/* HW4 - new add =================================================*/
static int
compareL1(Thread* t1, Thread* t2)
{
if ( t1->getBurstTime() > t2->getBurstTime() ) return 1;
else if ( t1->getBurstTime() < t2->getBurstTime() ) return -1;
else return t1->getID() < t2->getID() ? -1 : 1;
return 0;
}
static int
compareL2(Thread* t1, Thread* t2)
{
if ( t1->getPriority() > t2->getPriority() ) return -1;
else if( t1->getPriority() < t2->getPriority() ) return 1;
else return t1->getID() < t2->getID() ? -1 : 1;
return 0;
}
/*==================================================================*/

/* HW3 new add - but no use in HW4 */
// int pqSchedulingCompare (Thread *x, Thread *y)
// {
//     if (x->getpriority() < y->getpriority()) { return 1; } // x has higher priority
//     else if (x->getpriority() > y->getpriority()) { return -1; } // y has higher priority
//     else { return 0; } // priorities are equal
// }

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------
Scheduler::Scheduler()
{ 
    // readyList = new List<Thread *>; 
    // pqList = new SortedList<Thread*>(pqSchedulingCompare);
    L1ReadyList = new SortedList<Thread *>(compareL1);
    L2ReadyList = new SortedList<Thread *>(compareL2);
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

/* HW4 new add======================================================================= */
void Scheduler::updatePriority()
{
    ListIterator<Thread *> *iter1 = new ListIterator<Thread *>(L1ReadyList);
    ListIterator<Thread *> *iter2 = new ListIterator<Thread *>(L2ReadyList);
    ListIterator<Thread *> *iter3 = new ListIterator<Thread *>(L3ReadyList);
    Statistics *stats = kernel->stats;
    int oldPriority;
    int newPriority;

    // L1
    /* 有問題，沒有ReadyToRun()，有待觀察 */
    for( ; !iter1->IsDone(); iter1->Next() )
    {
        ASSERT( iter1->Item()->getStatus() == READY);
        iter1->Item()->setWaitingTime(iter1->Item()->getWaitingTime()+TimerTicks);
        if(iter1->Item()->getWaitingTime() >= 150 && iter1->Item()->getID() > 0 )
        {
            oldPriority = iter1->Item()->getPriority();
            newPriority = oldPriority + 10;
            if (newPriority > 149)
                newPriority = 149;

            iter1->Item()->setPriority(newPriority);
            iter1->Item()->setWaitingTime(0);
        }
    }

    // L2
    for( ; !iter2->IsDone(); iter2->Next() )
    {
        ASSERT( iter2->Item()->getStatus() == READY);
        iter2->Item()->setWaitingTime(iter2->Item()->getWaitingTime()+TimerTicks);
        if(iter2->Item()->getWaitingTime() >= 1500 && iter2->Item()->getID() > 0 )
        {
            oldPriority = iter2->Item()->getPriority();
            newPriority = oldPriority + 10;
            if (newPriority > 149)
                newPriority = 149;

            iter2->Item()->setPriority(newPriority);
            L2ReadyList->Remove(iter2->Item());
            ReadyToRun(iter2->Item());
        }
    }

    // L3
    for( ; !iter3->IsDone(); iter3->Next() )
    {
        ASSERT( iter3->Item()->getStatus() == READY);
        iter3->Item()->setWaitingTime(iter3->Item()->getWaitingTime()+TimerTicks);
        if( iter3->Item()->getWaitingTime() >= 1500 && iter3->Item()->getID() > 0 )
        {
            oldPriority = iter3->Item()->getPriority();
            newPriority = oldPriority + 10;
            if (newPriority > 149)
                newPriority = 149;

            iter3->Item()->setPriority(newPriority);
            L3ReadyList->Remove(iter3->Item());
            ReadyToRun(iter3->Item());
        }
    }
}
/*==================================================================================*/


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
    thread->setStatus(READY);

    // readyList->Append(thread);
    // pqList->Insert(thread);
    
    // if( !pqList->IsInList(thread) )
    // {
    //     DEBUG(dbtwo, "[A] Tick ["<< kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is inserted into queue")
    //     pqList->Insert(thread);
    // }

    /* HW4 new add ========================================================== */
    if(thread->getPriority() >= 100 && thread->getPriority() <= 149)
    {
        if( !kernel->scheduler->L1ReadyList->IsInList(thread) )
            L1ReadyList->Insert(thread);
    }
    else if ( (thread->getPriority() >= 50 && thread->getPriority() <= 99) )
    {
        if( !L2ReadyList->IsInList(thread) )
            L2ReadyList->Insert(thread);
    }
    else if ( (thread->getPriority() >= 0 && thread->getPriority() <= 49) )
    {
        if( !L3ReadyList->IsInList(thread) )    
            L3ReadyList->Append(thread);
    }
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

    if (pqList->IsEmpty()) {
		return NULL;
    } else {
    	return pqList->RemoveFront();
    }
    if (pqList->IsEmpty()) {
        return NULL;
    } else {
        DEBUG(dbtwo, "[B] Tick [" << kernel->stats->totalTicks << "]:Thread [" << pqList ->RemoveFront()->getID() << "] is removeed from queue");
        return pqList ->RemoveFront();
    }

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

    if (finishing) {	// mark that we need to delete current thread
         ASSERT(toBeDestroyed == NULL);
	 toBeDestroyed = oldThread;
    }
    
    if (oldThread->space != NULL) {	// if this thread is a user program,
        oldThread->SaveUserState(); 	// save the user's CPU registers
	oldThread->space->SaveState();
    }
    
    oldThread->CheckOverflow();		    // check if the old thread
					    // had an undetected stack overflow

    kernel->currentThread = nextThread;  // switch to the next thread
    nextThread->setStatus(RUNNING);      // nextThread is now running
    
    DEBUG(dbgThread, "Switching from : " << oldThread->getName() << " to: " << nextThread->getName());
    
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".
    DEBUG(dbtwo, "[D] Tick [" << kernel->stats->totalTicks << 
                 "]:Thread [" << nextThread->getID() << 
                 "] now selected for execution, thread [" <<
                 oldThread->getID() << 
                 "] is replaced, and it has executed [" << 
                 kernel->stats->totalTicks - kernel->stats->prevTicks << 
                 "] ticks");
    //cout<< oldThread->getName() << " context switch to " << nextThread->getName() << "\n";
    SWITCH(oldThread, nextThread);
    kernel->stats->prevTicks = kernel->stats->totalTicks;
    // SWITCH(oldThread, nextThread);

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
