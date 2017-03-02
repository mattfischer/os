#ifndef INIT_FS_H
#define INIT_FS_H

class Object;

/*!
 * \brief The InitFS filesystem that is built into the kernel
 */
class InitFs {
public:
	static void start();
	static int nameServer();
};

#endif
