#ifndef INIT_FS_H
#define INIT_FS_H

class Object;

/*!
 * \brief The InitFS filesystem that is built into the kernel
 */
class InitFs {
public:
	InitFs();

	int object();
	void start();

private:
	void server();
	static void serverStatic(void *param);

	int mObject;
	int mChannel;
};

#endif
