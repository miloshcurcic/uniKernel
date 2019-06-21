#include "pcb.h"
#include "sig_hand.h"

// HandlerNode:

HandlerNode::HandlerNode(SignalHandler handler, HandlerNode *next) {
	this->handler = handler;
	this->next = next;
}

// HandlerList:

HandlerList::HandlerList() {
	this->first_node = 0;
	this->last_node = 0;
	this->signal_blocked = 0;
}

void HandlerList::registerHandler(SignalHandler handler) {
	for(HandlerNode* cur = first_node; cur; cur=cur->next)
		if(cur->handler == handler)
			return;
	last_node = (first_node ? last_node->next : first_node) = new HandlerNode(handler);
}

void HandlerList::unregisterAllHandlers() {
	HandlerNode *cur = first_node;
	while(cur) {
		HandlerNode *temp = cur;
		cur = cur->next;
		delete temp;
	}
	first_node = last_node = 0;
}

void HandlerList::copyHandlers(const HandlerList &list) {
	this->signal_blocked = list.signal_blocked;
	for(HandlerNode *cur = list.first_node; cur; cur=cur->next)
		this->last_node = (this->first_node ? this->last_node->next : this->first_node) = new HandlerNode(cur->handler);
}

void HandlerList::swap(SignalHandler hand1, SignalHandler hand2) {
	HandlerNode *node1 = 0, *node2 = 0;
	for(HandlerNode *cur = first_node; cur; cur=cur->next) {
		if(!node1 && (cur->handler == hand1))
			node1 = cur;
		if(!node2 && (cur->handler == hand2))
			node2 = cur;
		if(node1 && node2)
			break;
	}
	if((!node1 || !node2) || (node1 == node2))
		return;

	node1->handler = hand2;
	node2->handler = hand1;
}

void HandlerList::callAllHandlers() {
	for(HandlerNode *cur = first_node; cur; cur=cur->next)
		cur->handler();
}

HandlerList::~HandlerList() {
	PCB::lock();
	unregisterAllHandlers();
	PCB::unlock();
}

// SignalNode:

void SignalNode::SignalNode(SignalId signal, SignalNode *next) {
	this->signal = signal;
	this->next = next;
}

// SignalController:

// Globally blocked signals:
volatile int SignalController::globally_blocked[totalSignals] = { 0 };

SignalController::SignalController() {
	first_node = last_node = 0;
}

void SignalController::registerHandler(SignalId signal, SignalHandler handler) {
	PCB::lock();
	if(handler != 0)
		myHandlers[signal].registerHandler(handler);
	PCB::unlock();
}

void SignalController::unregisterAllHandlers(SignalId signal) {
	PCB::lock();
	myHandlers[signal].unregisterAllHandlers();
	PCB::unlock();
}

void SignalController::swap(SignalId signal, SignalHandler hand1, SignalHandler hand2) {
	PCB::lock();
	myHandlers[signal].swap(hand1, hand2);
	PCB::unlock();
}

void SignalController::blockSignal(SignalId signal) {
	PCB::lock();
	myHandlers[signal].signal_blocked = 1;
	PCB::unlock();
}

void SignalController::unblockSignal(SignalId signal) {
	PCB::lock();
	myHandlers[signal].signal_blocked = 0;
	PCB::unlock();
}

void SignalController::blockSignalGlobally(SignalId signal) {
	PCB::lock();
	globally_blocked[signal] = 1;
	PCB::unlock();
}

void SignalController::unblockSignalGlobally(SignalId signal) {
	PCB::lock();
	globally_blocked[signal] = 0;
	PCB::unlock();
}

void SignalController::addSignal(SignalId signal) {
	PCB::lock();
	if(myHandlers[signal].first_node)
		last_node = (first_node ? last_node->next : first_node ) = new SignalNode(signal);
	PCB::unlock();
}

int SignalController::queue_empty() {
	return first_node == 0;
}

void SignalController::copyController(const SignalController &parent) {
	PCB::lock();
	for(int i=0;i<totalSignals;i++)
		myHandlers[i].copyHandlers(parent.myHandlers[i]);
	PCB::unlock();
}

void SignalController::callHandlers() { // Should be called from timer
	PCB::lock();
#ifndef BCC_BLOCK_IGNORE
	asm sti;
#endif
	while(first_node && !myHandlers[first_node->signal].signal_blocked && !globally_blocked[first_node->signal]) {
		SignalNode *temp = first_node;
		first_node = first_node->next;
		if(!first_node)
			last_node = 0;
		myHandlers[temp->signal].callAllHandlers();
		delete temp;
	}
	if(first_node) {
		SignalNode *new_last = first_node;
		SignalNode *cur = first_node->next;
		while(cur) {
			SignalNode *temp = cur;
			cur = cur->next;
			if(!myHandlers[temp->signal].signal_blocked && !globally_blocked[temp->signal]) {
				myHandlers[temp->signal].callAllHandlers();
				delete temp;
			} else {
				new_last->next = temp;
				temp->next = 0;
				new_last = new_last->next;
			}
		}
		last_node = new_last;
	} else last_node = 0;

#ifndef BCC_BLOCK_IGNORE
	asm cli;
#endif
	PCB::unlock();
}

SignalController::~SignalController() {
	PCB::lock();
	while(first_node) {
		SignalNode *removing = first_node;
		first_node = first_node->next;
		delete removing;
	}
	PCB::unlock();
}
