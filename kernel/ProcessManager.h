#ifndef PROC_MANAGER_H
#define PROC_MANAGER_H

#include "Object.h"

/*!
 * \brief Process services for userspace
 */
class ProcessManager {
public:
	static void start();

	/*!
	 * \brief Get the object representing the process manager
	 * \return Object id
	 */
	static Object *object();

private:
	static int sObject;

	static void main(void *param);
};



#endif