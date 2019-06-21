#ifndef _ker_kill_h_
#define _ker_kill_h_

#include "thread.h"
#include "pcb.h"
#include "semaphor.h"

class ThreadKiller : public Thread {
	// Constructors:
	ThreadKiller() : Thread(defaultStackSize, 0) {}
protected:
	// Methods:

	virtual void run();

private:

	static void add(PCB* toKill);

	// Destructor:

	~ThreadKiller();

	// Fields:

	static ThreadList clearing;
	static Semaphore *sem;
	static int done;

	// Friends:

	friend int main(int, char**);
	friend void sig0_handle();
};

#endif
