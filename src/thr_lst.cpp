#include "pcb.h"

// ~ ~ ~ This is a thread-safe structure ~ ~ ~

ThreadNode::ThreadNode(PCB *thread, ThreadNode* prev, ThreadNode* next) {
	PCB::lock();
	this->thread = thread;
	this->next = next;
	this->prev = prev;
	PCB::unlock();
}

void ThreadNode::unlinkNode(ThreadList *list) { // Correct list must be given here
	PCB::lock();
	if(this == list->first_thread)
		list->first_thread = list->first_thread->next;

	if(this == list->last_thread)
		list->last_thread = list->last_thread->prev;

	if(this->prev)
		this->prev->next = this->next;

	if(this->next)
		this->next->prev = this->prev;

	this->next = this->prev = 0;
	list->num_threads--;
	PCB::unlock();
}

ThreadList::ThreadList() {
	PCB::lock();
	this->num_threads = 0;
	this->first_thread = 0;
	this->last_thread = 0;
	PCB::unlock();
}

int ThreadList::threadsNum()  { return num_threads; }

PCB* ThreadList::getPCBbyID(ID id) {
	PCB::lock();
	ThreadNode *cur;
	for(cur=first_thread; cur && (id != cur->thread->myID); cur=cur->next);
	PCB* return_thread = (cur?cur->thread:0);
	PCB::unlock();
	return return_thread;
}

ThreadNode* ThreadList::queue_put(PCB* thread) {
	PCB::lock();
	ThreadNode* new_node = new ThreadNode(thread, last_thread);
	last_thread = (first_thread?last_thread->next:first_thread) = new_node;
	num_threads++;
	PCB::unlock();
	return new_node;
}

PCB* ThreadList::queue_get() {
	PCB::lock();
	ThreadNode* return_node = 0;
	if(first_thread) {
		return_node = first_thread;
		first_thread = first_thread->next;
		if(!first_thread)
			last_thread = 0;
		else
			first_thread->prev = 0;
		return_node->next = 0;
		num_threads--;
	}
	PCB* return_thread = (return_node?return_node->thread:0);
	if(return_node)
		delete return_node;
	PCB::unlock();
	return return_thread;
}

PCB* ThreadList::queue_first() {
	PCB::lock();
	PCB* return_thread = 0;
	if(first_thread)
		return_thread = first_thread->thread;
	PCB::unlock();
	return return_thread;
}

ThreadList::~ThreadList() {
	PCB::lock();
	ThreadNode* clearing_node = first_thread;
	while(clearing_node) {
		first_thread = first_thread->next;
		delete clearing_node;
		clearing_node = first_thread;
	}
	first_thread = last_thread = 0;
	num_threads = 0;
	PCB::unlock();
}
