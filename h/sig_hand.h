#ifndef _sig_hand_h_
#define _sig_hand_h_

#include "define.h"

struct HandlerNode {
	// Constructors:

	HandlerNode(SignalHandler handler, HandlerNode *next = 0);

private:

	// Fields:

	volatile SignalHandler handler;
	HandlerNode *volatile next;

	// Friends:

	friend class HandlerList;
};

class HandlerList {
	// Constructors:

	HandlerList();

	// Methods:

	void registerHandler(SignalHandler handler);
	void unregisterAllHandlers();
	void copyHandlers(const HandlerList &list);
	void swap(SignalHandler hand1, SignalHandler hand2);

	// Desctructor:

	~HandlerList();

	// Control methods:

	void callAllHandlers();

	// Fields:

	HandlerNode * volatile first_node, * volatile last_node;
	volatile int signal_blocked;

	// Friends:

	friend class SignalController;
};

struct SignalNode {
	// Constructors:

	SignalNode(SignalId signal, SignalNode *next = 0);

	// Fields:

	SignalId signal;
	SignalNode *volatile next;

	// Friends:

	friend class SignalController;

};

class SignalController {
	// Constructors:

	SignalController();

	// Methods:

	// ~ Handlers:

	void registerHandler(SignalId signal, SignalHandler handler);
	void unregisterAllHandlers(SignalId signal);
	void swap(SignalId signal, SignalHandler hand1, SignalHandler hand2);
	void blockSignal(SignalId signal);
	static void blockSignalGlobally(SignalId signal);
	void unblockSignal(SignalId signal);
	static void unblockSignalGlobally(SignalId signal);
	void copyController(const SignalController &parent);

	// ~ Signal queue:

	void addSignal(SignalId signal);
	int queue_empty();

	// ~ Control methods:

	void callHandlers();

	// Destructor:

	~SignalController();

	// Fields:

	volatile static int globally_blocked[totalSignals];
	SignalNode *volatile first_node;
	SignalNode *volatile last_node;
	HandlerList myHandlers[totalSignals];

	// Friends:

	friend class PCB;
	friend class Thread;
	friend void interrupt timer(...);
};


#endif
