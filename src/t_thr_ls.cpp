#include "t_thr_ls.h"
#include "pcb.h"

// ~ ~ ~ Not a thread-safe structure ~ ~ ~

// TimedThreadNode:
TimedThreadNode::TimedThreadNode(PCB* thread, Time time, TimedThreadNode* prev, TimedThreadNode* next) {
	this->thread = thread;
	this->maxWait = time;
	this->next = next;
	this->prev = prev;
	this->next_timed = 0;
	this->prev_timed = 0;
}

// TimedThreadList:
TimedThreadList::TimedThreadList() {
	this->first_thread = 0;
	this->last_thread = 0;
	this->first_timed_node = 0;
}

PCB* TimedThreadList::timed_queue_get() {
	TimedThreadNode* return_node = 0;
	if(first_thread) {
		// Take the first node from the thread list:
		return_node = first_thread;
		first_thread = first_thread->next;
		if(!first_thread)
			last_thread = 0;
		else
			first_thread->prev = 0;
		return_node->next = 0;
		// Remove this node from the timed list:
		if(return_node->maxWait) {
			if(return_node==first_timed_node)
				first_timed_node = first_timed_node->next_timed;
			if(return_node->next_timed)
				return_node->next_timed->prev_timed = return_node->prev_timed;
			if(return_node->prev_timed) {
				return_node->prev_timed->next_timed = return_node->next_timed;
			}
		}
	}
	PCB* return_thread = (return_node?return_node->thread:0);
	if(return_node) {
		PCB::lock();
		delete return_node;
		PCB::unlock();
	}
	if(return_thread)
		return_thread->sem_flag = 1;
	return return_thread;
}

void TimedThreadList::t_t_list_tick() {
	// If the first thread maxWait value has become
	// zero - resume it and all successive threads
	// after it with the maxWait value of 0
	if(first_timed_node && !(--first_timed_node->maxWait)) {
		while(first_timed_node && !first_timed_node->maxWait) {
			// Remove node from the timed list:
			TimedThreadNode* timer_done = first_timed_node;
			first_timed_node = first_timed_node->next_timed;
			if(first_timed_node)
				first_timed_node->prev_timed = 0;
			// Remove node from the regular list:
			if(timer_done == first_thread)
				first_thread = first_thread->next;
			if(timer_done == last_thread)
				last_thread = last_thread->prev;
			if(timer_done->prev)
				timer_done->prev->next = timer_done->next;
			if(timer_done->next)
				timer_done->next->prev = timer_done->prev;
			// Put taken node in the scheduler:
			if(timer_done->thread->myState == BLOCKED) {
				timer_done->thread->myState = READY;
				timer_done->thread->sem_flag = 0;
				Scheduler::put(timer_done->thread);
			}

			delete timer_done;
		}
	}
}

void TimedThreadList::timed_put(PCB* thread, Time maxWait) {
	PCB::lock();
	TimedThreadNode *new_node = new TimedThreadNode(thread, maxWait, last_thread);
	PCB::unlock();
	last_thread = (first_thread?last_thread->next:first_thread) = new_node;
	// Put this node in the timed list:
	if(maxWait) {
		TimedThreadNode *cur = first_timed_node;
		TimedThreadNode *prev = 0;
		while(cur && (new_node->maxWait >= cur->maxWait)) {
			new_node->maxWait -= cur->maxWait;
			prev = cur;
			cur = cur->next_timed;
		}
		new_node->next_timed = cur;
		new_node->prev_timed = prev;
		if(cur) {
			cur->maxWait -= new_node->maxWait;
			cur->prev_timed = new_node;
		}
		if(prev)
			prev->next_timed = new_node;
		else
			first_timed_node = new_node;
	}
}
