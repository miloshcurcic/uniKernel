#ifndef _pcb_h_
#define	_pcb_h_

#include "define.h"
#include "thr_lst.h"
#include "sig_hand.h"

class PCB {

	// Constructors:

	PCB(Thread* thread, StackSize stackSize, Time timeSlice);

	// Methods:

	// ~ Initialize PCB (allocate and setup the stack):
	static void initPCB(PCB* pcb);
	// ~ Wrapper function for thread execution:
	static void wrapper(void* myArg);
	// ~ Notify threads that are waiting for me to complete:
	void notifyWaitingThreads();

public:
	// ~ Context change lock and unlock:
	static void lock();
	static void unlock();

private:
	// Destructor:

	~PCB();

	// Fields:

	// Saved registers:

	volatile unsigned sp; // Stack Pointer (Offset)
	volatile unsigned ss; // Stack Segment

	volatile unsigned bp; // Base Pointer

	// Control fields:

	static ID SID;

	ID myID;
	StackSize myStackSize;
	Time myTimeSlice;
	ThreadState myState;

	// Pointer to the top of the stack:

	unsigned* myStack;

	// Pointer to node in the containing thread list:

	ThreadNode *myControlNode;

	// Pointer to myThread object that has initiated this thread:

	Thread *myThread;

	// My parent thread:

	PCB* myParent;

	// Semaphores:

	// Semaphore unblock flag:

	volatile int sem_flag;

	// Signals:

	SignalController mySignals;


	// List of threads waiting for me to finish:

	ThreadList* waiting;

	// Global control structures:

	// Running thread:

	static PCB* volatile running_thread;
	volatile static int running_thread_proc_time;

	// Context switching:

	volatile static int context_locked;
	volatile static int explicit_context_change_requested;
	volatile static int context_change_pending;

	// List of all created threads that still have their objects:

	static ThreadList *threads;


	// Friends:

	// ~ Friend classes:

	friend class Thread;
	friend class ThreadList;
	friend class TimedThreadList;
	friend class KernelEv;
	friend class KernelSem;
	friend class ThreadKiller;
	friend class SignalController;

	// debugging
	friend class Producer;

	// ~ Friend functions:

	friend void interrupt timer(...);
	friend void kernel_init();
	friend void clearCollectorStack();
	friend void sig0_handle();
	friend void dispatch();
	friend void clearAllocatedData();

};

#endif
