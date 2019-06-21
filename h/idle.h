#ifndef _idle_h_
#define _idle_h_

class Idle : public Thread {
public:
	Idle() : Thread(defaultStackSize, 1) {
		finished = 0;
	}
	~Idle() {
		waitToComplete();
	}
protected:
	virtual void run() {
		while(!finished);
	}
private:
	int finished;
	friend int main(int, char**);
};

#endif
