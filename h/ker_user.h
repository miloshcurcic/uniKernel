#ifndef _user_h_
#define _user_h_

#include "define.h"
#include "thread.h"

class KernelUser : public Thread {
public:
	KernelUser(int argc, char** argv) {
		this->argc = argc;
		this->argv = argv;
		this->result = 0;
	}
	int result;
	~KernelUser() { waitToComplete(); }
private:
	void run() {
		result = userMain(argc, argv);
	}
	int argc;
	char** argv;
};

#endif
