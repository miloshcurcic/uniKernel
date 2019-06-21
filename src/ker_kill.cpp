#include "ker_kill.h"

// ThreadKiller:

Semaphore *ThreadKiller::sem = new Semaphore(0);
ThreadList ThreadKiller::clearing;
int ThreadKiller::done = 0;

void ThreadKiller::run() {
	while(!done) {
		sem->wait(0);
		PCB *toKill = clearing.queue_get();
		PCB::lock();

		if(toKill && toKill->myState != FINISHED) {
			if(toKill->waiting)
				toKill->notifyWaitingThreads();
			delete toKill->myStack;
			toKill->myStack = 0;
			delete toKill->waiting;
			toKill->waiting = 0;
			toKill->myState = FINISHED;
		}
		PCB::unlock();
	}
	PCB *toKill = 0;
	while ((toKill = clearing.queue_get()) && toKill->myState != FINISHED) {
		toKill->notifyWaitingThreads();
		delete toKill->myStack;
		toKill->myStack = 0;
		delete toKill->waiting;
		toKill->waiting = 0;
		toKill->myState = FINISHED;
	}
}

void ThreadKiller::add(PCB* toKill) {
	PCB::lock();
	clearing.queue_put(toKill);
	sem->signal();
	PCB::unlock();

}

ThreadKiller::~ThreadKiller() {
	waitToComplete();
	PCB::lock();
	delete sem;
	PCB::unlock();
}
void sig0_handle() {
	PCB::running_thread->myState = DYING;
	ThreadKiller::add(PCB::running_thread);
}
