#include "kern_ev.h"
#include "pcb.h"

void dispatch();

// IVTEntry

IVTEntry* IVTEntry::all_entries[totalIVTEntries] = { 0 };


IVTEntry::IVTEntry(IVTNo ivtNo, pInterrupt myInterrupt) {
	if(!IVTEntry::all_entries[ivtNo]) {
		this->myIVTNo = ivtNo;
		this->myInterrupt = myInterrupt;
		IVTEntry::all_entries[ivtNo] = this;
#ifndef BCC_BLOCK_IGNORE
		this->oldInterrupt = getvect(ivtNo);
		setvect(ivtNo, this->myInterrupt);
#endif
	}
}

IVTEntry* IVTEntry::getIVTEntry(IVTNo ivtNo) {
	return IVTEntry::all_entries[ivtNo];
}

void IVTEntry::signal() {
	if(KernelEv::all_events[this->myIVTNo]) {
		KernelEv::all_events[this->myIVTNo]->signal();
		dispatch();
	}
}

void IVTEntry::callOld() {
	this->oldInterrupt();
}

IVTEntry::~IVTEntry() {
	IVTEntry::all_entries[this->myIVTNo] = 0;
	// this->oldInterrupt(); // This might be needed as the keyboard sometimes bugs out
#ifndef BCC_BLOCK_IGNORE
	setvect(this->myIVTNo, this->oldInterrupt);
#endif
}


// KernelEv:

KernelEv* KernelEv::all_events[totalIVTEntries] = { 0 };

KernelEv::KernelEv(IVTNo ivtNo) {
	this->myIVTNo = ivtNo;
	this->value = 0;
	this->isBlocked = 0;
	KernelEv::all_events[ivtNo] = this;
	this->created_by = PCB::running_thread;
}


void KernelEv::wait() {
	PCB::lock();
	if(PCB::running_thread == this->created_by)
		if(!this->value) {
			PCB::running_thread->myState = BLOCKED;
			this->isBlocked = 1;
			PCB::unlock();
			dispatch();
			return;
		} else
			this->value = 0;
	PCB::unlock();
}

void KernelEv::signal() {
	PCB::lock();
	if(!this->isBlocked)
		this->value = 1;
	else {
		if(this->created_by->myState == BLOCKED) {
			this->created_by->myState = READY;
			this->isBlocked = 0;
			Scheduler::put(this->created_by);
		}
	}
	PCB::unlock();
}

KernelEv::~KernelEv () {
	KernelEv::all_events[myIVTNo] = 0;
}
