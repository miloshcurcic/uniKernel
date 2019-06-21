#ifndef _kern_ev_h
#define _kern_ev_h

#include "define.h"

// IVTEntry is in define.h

class KernelEv {

	// Constructors:

	KernelEv (IVTNo ivtNo);

	// Methods:

	void wait ();

	// Destructor:

	~KernelEv ();

protected:

	void signal(); // can call KernelEv

private:

	// Fields:

	IVTNo myIVTNo;

	int volatile value;
	int volatile isBlocked;

	PCB* created_by;

	// Array of all Kernel Events:

	static KernelEv* all_events[totalIVTEntries];

	// Friends:
	friend class Event;
	friend class IVTEntry;
};


#endif
