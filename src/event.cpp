#include "event.h"
#include "kern_ev.h"
#include "pcb.h"

Event::Event(IVTNo ivtNo) {
	PCB::lock();
	this->myImpl = new KernelEv(ivtNo);
	PCB::unlock();
}

void Event::wait() {
	myImpl->wait();
}

void Event::signal() {
	myImpl->signal();
}

Event::~Event() {
	PCB::lock();
	delete myImpl;
	PCB::unlock();
}

