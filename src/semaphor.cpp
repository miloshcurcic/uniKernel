#include "semaphor.h"
#include "kern_sem.h"
#include "pcb.h"

Semaphore::Semaphore(int init) {
	PCB::lock();
	myImpl = new KernelSem(init);
	PCB::unlock();
}

int Semaphore::wait(Time maxTimeToWait) {
	return myImpl->wait(maxTimeToWait);
}

int Semaphore::signal(int n) {
	return myImpl->signal(n);
}

int Semaphore::val() const {
	return myImpl->val();
}

Semaphore::~Semaphore() {
	PCB::lock();
	delete myImpl;
	PCB::unlock();
}
