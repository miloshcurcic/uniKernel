#include "pcb.h"
#include "thread.h"
#include "kern_sem.h"

// PCB:

// ~ Global fields:

// ~ ~ Thread ID:
ID PCB::SID = 0;
// ~ ~ Context control:
volatile int PCB::context_locked = 0;
volatile int PCB::explicit_context_change_requested = 0;
volatile int PCB::context_change_pending = 0;
// ~ ~ Running thread:
PCB* volatile PCB::running_thread = new PCB(0, 0, 0); // Used to capture the kernel main thread as thread_0
volatile int PCB::running_thread_proc_time = -1; // Kernel thread will be running until dispatch() is called (this thread is supposed to wait for the user thread to finish)
// ~ ~ List of all created threads that still have their objects:
ThreadList *PCB::threads = new ThreadList();

// ~ Methods

// Is always called from safe environment, no need for context locking:
PCB::PCB(Thread* thread, StackSize stackSize, Time timeSlice) {
	this->waiting = new ThreadList();
	this->myThread = thread;
	this->myStackSize = stackSize/sizeof(unsigned);
	if (this->myStackSize > maxStackSize)
		this->myStackSize = maxStackSize;
	this->myTimeSlice = timeSlice;
	this->myID = SID++;
	if(this->myID != 0) {
		this->myState = INIT;
		this->myControlNode = PCB::threads->queue_put(this);
		this->myParent = PCB::running_thread;
		this->mySignals.copyController(PCB::running_thread->mySignals);
	} else {
		this->myState = RUNNING;
		this->myParent = 0;
		this->myControlNode = 0;
	}
	this->myStack = 0;
	this->sem_flag = 0;
}

// Is always called from safe environment, no need for context locking:
void PCB::initPCB(PCB* pcb) {
	pcb->myStack = new unsigned[pcb->myStackSize];
	#ifndef BCC_BLOCK_IGNORE
	pcb->myStack[pcb->myStackSize-1] = FP_SEG(pcb->myThread);
	pcb->myStack[pcb->myStackSize-2] = FP_OFF(pcb->myThread);
#endif
	pcb->myStack[pcb->myStackSize-5] = 0x200;
#ifndef BCC_BLOCK_IGNORE
	pcb->myStack[pcb->myStackSize-6] = FP_SEG(PCB::wrapper);
	pcb->myStack[pcb->myStackSize-7] = FP_OFF(PCB::wrapper);
	pcb->ss = FP_SEG(pcb->myStack + pcb->myStackSize - 16);
	pcb->sp = FP_OFF(pcb->myStack + pcb->myStackSize - 16);
#endif
	pcb->bp = pcb->sp;
	pcb->myState=READY;
}

// Is always called from safe environment, no need for context locking:
void PCB::notifyWaitingThreads() {
	PCB *waiting = 0;
	while(1) {
		waiting = PCB::waiting->queue_get();
		if(!waiting)
			break;
		waiting->myState = READY;
		Scheduler::put(waiting);
	}
}

// myArg is open for reuse as it is not needed in this implementation
// There's also a possibility to make dispatch() a callback function through
// adding it's address to the stack in initPCB()
void PCB::wrapper(void* myArg) {
	PCB::running_thread->myThread->run();
	if(PCB::running_thread->myParent->myThread)
		PCB::running_thread->myParent->myThread->signal(1);
	PCB::running_thread->myThread->signal(2);
	while(!PCB::running_thread->mySignals.queue_empty()) // Should I wait for this or not?
		dispatch();
	PCB::lock();
	PCB::running_thread->myState = FINISHED;
	PCB::running_thread->notifyWaitingThreads();
	PCB::unlock();
	dispatch();
}

void PCB::lock() {
	context_locked++;
}

void PCB::unlock() {
	context_locked--;
}

// Should be locked:
PCB::~PCB() {
	PCB::lock();
	if(this->myControlNode)
		this->myControlNode->unlinkNode(PCB::threads);
	delete this->myControlNode;
	this->myControlNode = 0;
	delete this->myStack;
	this->myStack = 0;
	delete this->waiting;
	this->waiting = 0;
	PCB::unlock();
}

// Timer:
volatile unsigned tsp = 0, tss = 0, tbp = 0;

void interrupt timer(...) {

	if(!PCB::explicit_context_change_requested) {
		oldRoutine();
		tick();
		KernelSem::kern_sem_tick();
		if(PCB::running_thread_proc_time>0)
			PCB::running_thread_proc_time--;
	}

	if(!PCB::running_thread_proc_time || PCB::explicit_context_change_requested || PCB::context_change_pending) {
		if(!PCB::context_locked) {

			PCB::running_thread->mySignals.callHandlers();

			PCB::explicit_context_change_requested = 0;
			PCB::context_change_pending = 0;
#ifndef BCC_BLOCK_IGNORE
			asm {
				mov word ptr tsp, sp
				mov word ptr tss, ss
				mov word ptr tbp, bp
			}
#endif
			PCB::running_thread->sp = tsp;
			PCB::running_thread->ss = tss;
			PCB::running_thread->bp = tbp;

			if(PCB::running_thread->myState == RUNNING) {
				PCB::running_thread->myState = READY;
				Scheduler::put(PCB::running_thread);
			}

			PCB::running_thread = Scheduler::get();

			PCB::running_thread->myState = RUNNING;

			tsp = PCB::running_thread->sp;
			tss = PCB::running_thread->ss;
			tbp = PCB::running_thread->bp;

			PCB::running_thread_proc_time = (PCB::running_thread->myTimeSlice > 0)?PCB::running_thread->myTimeSlice:-1;

#ifndef BCC_BLOCK_IGNORE
			asm {
				mov sp, word ptr tsp
				mov ss, word ptr tss
				mov bp, word ptr tbp
			}
#endif

		} else {
			PCB::explicit_context_change_requested = 0;
			PCB::context_change_pending = 1;
		}
	}

}
