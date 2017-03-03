#ifndef SERVER_H
#define SERVER_H

class Object;

/*!
 * \brief Process services for userspace
 */
class Server {
public:
	Server();

	int startUserProcess(const char *cmdline, int stdinObject, int stdoutObject, int stderrObject, int nameserverObject);
	void run();

private:
	int mChannel;
	int mKernelObject;
};

#endif
