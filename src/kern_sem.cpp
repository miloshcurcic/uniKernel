#include "kern_sem.h"
#include"t_thr_ls.h"
#include "pcb.h"
#include "thread.h"

// KernelSemNode:

KernelSemNode::KernelSemNode(KernelSem* sem) {
	this->sem = sem;
	this->next = 0;
}

// KernelSem:

KernelSemNode *KernelSem::sem_list = 0;

// Is always called from safe environment, no need for context locking:
KernelSem::KernelSem(int init = 1) {
	this->value = init;
	this->blocked = new TimedThreadList();
	KernelSemNode* new_sem = new KernelSemNode(this);
	new_sem->next = sem_list;
	sem_list = new_sem;
}

int KernelSem::wait(Time maxTimeToWait) {
	PCB::lock();
	PCB::running_thread->sem_flag = 0;
	if(--value<0) {
		block(maxTimeToWait); // PCB::unlock() is called in this function (no deadlock)
		return PCB::running_thread->sem_flag;
	}
	PCB::unlock();
	return PCB::running_thread->sem_flag;
}

int KernelSem::signal(int num) {
	if (num>=0) {
		int res = -1, i = 0;
		if (!num) {
			num = 1;
			res = 0;
		}
		PCB::lock();
		if(value < 0) {
			if(-value > num)
				i = num;
			else
				i = -value;
			if(res == -1)
				res = i;
		}
		while(i-- > 0)
			deblock();
		value+=num;
		PCB::unlock();
		return res;
	}
	return num;
}

void KernelSem::block(Time maxTimeToWait) {
	blocked->timed_put(PCB::running_thread, maxTimeToWait);
	PCB::running_thread->myState = BLOCKED;
	PCB::unlock();
	dispatch();
}

void KernelSem::deblock() {
	PCB *resuming_thread = blocked->timed_queue_get();
	if(resuming_thread->myState == BLOCKED) {
		resuming_thread->myState = READY;
		Scheduler::put(resuming_thread);
	}
}

int KernelSem::val() const {
	return value;
}


void KernelSem::kern_sem_tick() {
	PCB::lock();
	KernelSemNode *cur = sem_list;
	while(cur) {
		cur->sem->blocked->t_t_list_tick();
		cur=cur->next;
	}
	PCB::unlock();
}

KernelSem::~KernelSem() {
	PCB::lock();
	while(value++ < 0)
		deblock();
	delete this->blocked;
	this->blocked = 0;
	KernelSemNode *cur = sem_list;
	KernelSemNode *prev = 0;
	while(cur && cur->sem != this) {
		prev = cur;
		cur = cur->next;
	}
	if(cur && prev) {
		prev->next = cur->next;
		delete cur;
	} else if (cur) {
		sem_list = sem_list->next;
		delete cur;
	}
	PCB::unlock();
}
