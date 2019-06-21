#ifndef _kern_sem_h_
#define _kern_sem_h_

#include "define.h"

class KernelSemNode {
	// Constructors:

	KernelSemNode(KernelSem* sem);

	// Fields:

	KernelSem *sem;
	KernelSemNode *next;

	// Friends:

	friend class KernelSem;
};

class KernelSem {
	// Constructors:

	KernelSem(int init);

	// Methods:

	int wait (Time maxTimeToWait);
	int signal(int num);
	int val() const;

	// Destructor:

	~KernelSem();

	// Auxiliary methods:

	void block(Time maxTimeToWait);
	void deblock();

	// Timer methods:

	static void kern_sem_tick();

	// Fields:

	volatile int value;
	TimedThreadList *blocked;

	// List of all semaphores:

	static KernelSemNode *sem_list;

	// Friends:

	friend class Semaphore;
	friend void interrupt timer(...);

};

#endif
