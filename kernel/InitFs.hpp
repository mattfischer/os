#ifndef INIT_FS_H
#define INIT_FS_H

class Object;

/*!
 * \brief The InitFS filesystem that is built into the kernel
 */
class InitFs {
public:
	static void start();
	static void setNameServer(Object* nameServer);
	static Object *nameServer();

private:
	static Object *sNameServer;
};

#endif
