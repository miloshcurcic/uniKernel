#ifndef _t_thr_ls_h_
#define _t_thr_ls_h_

#include "define.h"

struct TimedThreadNode {
	TimedThreadNode(PCB* thread, Time time, TimedThreadNode* prev = 0, TimedThreadNode* next = 0);
private:
	PCB* thread;
	volatile Time maxWait;
	TimedThreadNode* volatile next;
	TimedThreadNode* volatile prev;
	TimedThreadNode* volatile next_timed;
	TimedThreadNode* volatile prev_timed;

	// Friends:
	friend class TimedThreadList;
};

// This is a list within the actual thread list.
// In order to implement timed thread release
// in the most effective manner in this list
// we have time differences between the waiting
// threads. For example:
// If the threads are waiting for 5 8 8 9 *55ms
// respectively, this list will have elements 5 3 0 1

class TimedThreadList {
public:
	// Constructors:
	TimedThreadList();
	// Methods:
	void timed_put(PCB* thread, Time maxWait);
	PCB* TimedThreadList::timed_queue_get();
private:
	// Timer function
	void t_t_list_tick();
	// Fields
	TimedThreadNode* volatile first_thread;
	TimedThreadNode* volatile last_thread;
	TimedThreadNode* volatile first_timed_node;
	// Friends:
	friend class KernelSem;
};

#endif
