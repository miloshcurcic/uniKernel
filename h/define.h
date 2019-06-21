#ifndef _define_h_
#define _define_h_

#include <dos.h>
#include "SCHEDULE.h"

// Different types:
typedef unsigned long StackSize;
typedef unsigned int Time; // time, x 55ms
typedef int ID;
// Events:
typedef unsigned char IVTNo;
typedef void interrupt (*pInterrupt)(...);
// Signals:
typedef void (*SignalHandler)();
typedef unsigned SignalId;

// Class existence:
class PCB;
class Thread;
class ThreadList;
class TimedThreadList;
class KernelSem;
class KernelEv;

// Constants:
const int totalIVTEntries = 256;
const int totalSignals = 16;
const StackSize maxStackSize = 65536;
// Thread states:

enum ThreadState {
	RUNNING, INIT, READY, BLOCKED, DYING, FINISHED
};


// Functions:

int userMain(int argc, char* argv[]);
void tick();
void interrupt timer(...);
void checkSignals();
void sig0_handle();

// Saved timer routine:

extern pInterrupt oldRoutine;

// Events PREPAREENTRY macro:

class IVTEntry {
public:
	IVTEntry::IVTEntry(IVTNo ivtNo, pInterrupt myInterrupt);
	void callOld();
	void signal();
	~IVTEntry();

	static IVTEntry* getIVTEntry(IVTNo ivtNo);
private:
	IVTNo myIVTNo;
	pInterrupt volatile myInterrupt;
	pInterrupt volatile oldInterrupt;
	static IVTEntry* all_entries[totalIVTEntries];

	// Friends:
	friend class KernelEv;
};

#define PREPAREENTRY(ivtNo, callOldRoutine) \
void interrupt inter##ivtNo(...); \
IVTEntry ivtEntry##ivtNo(ivtNo, inter##ivtNo); \
void interrupt inter##ivtNo(...) { \
	ivtEntry##ivtNo.signal(); \
	if(callOldRoutine) \
		ivtEntry##ivtNo.callOld(); \
}

#endif
