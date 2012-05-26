#ifndef SCHED_H
#define SCHED_H

#include "List.hpp"

class Task;

/*!
 * \brief Task scheduler
 */
class Sched {
public:
	static void runNext();
	static void add(Task *task);
	static void switchTo(Task *task);
	static void setCurrent(Task *task);

	/*!
	 * \brief Currently-running task
	 * \return Current task
	 */
	static Task *current() { return sCurrent; }
private:
	static Task *sCurrent;
	static List<Task> sRunList;
};

#endif
