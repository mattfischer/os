#ifndef PROC_MANAGER_H
#define PROC_MANAGER_H

class ProcessManager {
public:
	static void start();

	static int object() { return sObject; }

private:
	static int sObject;

	static void main(void *param);
};



#endif