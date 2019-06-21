#include <iostream.h>

#include "ker_user.h"
#include "ker_kill.h"
#include "pcb.h"
#include "thread.h"
#include "idle.h"

pInterrupt oldRoutine = 0;

void kernel_init() {
#ifndef BCC_BLOCK_IGNORE
	oldRoutine = getvect(0x08);
	setvect(0x08, timer);
#endif
}

void kernel_restore() {
#ifndef BCC_BLOCK_IGNORE
	setvect(0x08, oldRoutine);
#endif
}

void clearAllocatedData() {
	delete PCB::threads;
	delete PCB::running_thread;
}

int main(int argc, char* argv[]) {

	// Initialize the kernel:

	kernel_init();

	// Setup the KernelUser thread and add the default handler for it
	// This handler will be inherited from the parent class

	KernelUser user(argc, argv);
	user.registerHandler(0, sig0_handle);

	// Idle thread:

	Idle idle;

	// ThreadKiller thread:

	ThreadKiller killer;

	// Start KernelUser thread, Idle thread and the ThreadKiller thread:

	user.start();
	idle.start();
	killer.start();

	// Wait for KernelUser thread to complete and collect the execution result:

	user.waitToComplete();
	int result = user.result;


	// Kill ThreadKiller thread:

	killer.done = 1;
	killer.sem->signal();	// Wake it up if it's sleeping
	killer.waitToComplete();

	// Kill the idle thread:

	idle.finished = 1;
	idle.waitToComplete();

	// Restore the system state before the kernel has been executed:

	kernel_restore();

	// Clearing other allocated data:

	clearAllocatedData();

	// The end! :D

	cout << "Happy end!\n";

	return result;
}
