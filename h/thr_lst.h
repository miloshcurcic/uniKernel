#ifndef _thr_lst_h_
#define _thr_lst_h_

#include "define.h"

struct ThreadNode {
	ThreadNode(PCB* thread, ThreadNode* prev = 0, ThreadNode* next = 0);
	void unlinkNode(ThreadList *list);
protected:
	PCB* thread;
	ThreadNode * volatile next, * volatile prev;
	// Friends:
	friend class ThreadList;
};

class ThreadList {
public:
	// Constructors:
	ThreadList();
	// Methods:
	PCB* getPCBbyID(ID id);
	int threadsNum();
	ThreadNode* queue_put(PCB* thread);
	PCB* queue_get();
	PCB* queue_first();
	// Desctructor:
	~ThreadList();

private:
	// Fields:
	ThreadNode * volatile first_thread, * volatile last_thread;
	volatile int num_threads;
	// Friends:
	friend class ThreadNode;
};

#endif
