#include "pcb.h"
#include "thread.h"

Thread::Thread(StackSize stackSize, Time timeSlice) {
	if (stackSize == 0)
		stackSize = defaultStackSize;
	if (stackSize > maxStackSize)
		stackSize = maxStackSize;
	PCB::lock();
	myPCB = new PCB(this, stackSize, timeSlice);
	PCB::unlock();
}

void Thread::start() {
	if(myPCB->myState == INIT) {
		PCB::lock();
		PCB::initPCB(myPCB);
		Scheduler::put(myPCB);
		PCB::unlock();
	}
}

void Thread::waitToComplete() {
	// Waiting on threads that have not yet been initialized, that have finished their execution and waiting on myself makes no sense
	if (myPCB->myState == READY || myPCB->myState == BLOCKED) {
		PCB::lock();
		myPCB->waiting->queue_put(PCB::running_thread);
		PCB::running_thread->myState = BLOCKED;
		PCB::unlock();
		dispatch(); // dispatch() doesn't return the thread in Scheduler if the state has been changed from RUNNING to something else
	}
}

Thread::~Thread() {
	waitToComplete();
	delete myPCB;
	myPCB = 0;
}

ID Thread::getId() {
	return myPCB->myID;
}

ID Thread::getRunningId() {
	return PCB::running_thread->myID;
}

Thread* Thread::getThreadById(ID id) {
	PCB* resultingThread = PCB::threads->getPCBbyID(id);
	if(resultingThread)
		return resultingThread->myThread;
	return 0;

}

void dispatch() {
#ifndef BCC_BLOCK_IGNORE
	asm cli;
#endif
	PCB::explicit_context_change_requested = 1;
	timer();
#ifndef BCC_BLOCK_IGNORE
	asm sti;
#endif
}

void Thread::signal(SignalId signal) {
	myPCB->mySignals.addSignal(signal);
}

void Thread::registerHandler(SignalId signal, SignalHandler handler) {
	myPCB->mySignals.registerHandler(signal, handler);
}

void Thread::unregisterAllHandlers(SignalId signal) {
	myPCB->mySignals.unregisterAllHandlers(signal);
}

void Thread::swap(SignalId id, SignalHandler hand1, SignalHandler hand2) {
	myPCB->mySignals.swap(id, hand1, hand2);
}

void Thread::blockSignal(SignalId signal) {
	myPCB->mySignals.blockSignal(signal);
}

void Thread::blockSignalGlobally(SignalId signal) {
	SignalController::blockSignalGlobally(signal);
}

void Thread::unblockSignal(SignalId signal) {
	myPCB->mySignals.unblockSignal(signal);
}

void Thread::unblockSignalGlobally(SignalId signal) {
	SignalController::unblockSignalGlobally(signal);
}
