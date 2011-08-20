#ifndef PROC_MANAGER_H
#define PROC_MANAGER_H

#include "Object.h"

class ProcessManager {
public:
	static void start();

	static int object() { return sObject; }

private:
	static int sObject;

	static void main(void *param);
};



#endif