#include <System.h>
#include <Message.h>
#include <Object.h>

#include <kernel/include/MessageFmt.h>
#include <kernel/include/KernelFmt.h>
#include <kernel/include/ProcessFmt.h>
#include <kernel/include/Objects.h>

#include <string.h>
#include <stddef.h>

int SpawnProcess(const char *argv[], int stdinObject, int stdoutObject, int stderrObject)
{
	return SpawnProcessx(argv, stdinObject, stdoutObject, stderrObject, NAMESERVER_NO);
}

int SpawnProcessx(const char *argv[], int stdinObject, int stdoutObject, int stderrObject, int nameserverObject)
{
	struct KernelMsg msg;
	int child;
	int i;
	char *c;

	msg.type = KernelSpawnProcess;

	// Construct the command line out of the passed-in argv
	c = msg.spawn.cmdline;
	for(i=0; argv[i] != NULL; i++) {
		strcpy(c, argv[i]);
		c += strlen(argv[i]) + 1;
	}
	*c = '\0';

	msg.spawn.stdinObject = stdinObject;
	msg.spawn.stdoutObject = stdoutObject;
	msg.spawn.stderrObject = stderrObject;
	msg.spawn.nameserverObject = nameserverObject;

	int objectsOffset = offsetof(struct KernelMsg, spawn.stdinObject);
	Object_Sendhs(KERNEL_NO, &msg, sizeof(msg), objectsOffset, 4, &child, sizeof(child));
	return child;
}

void WaitProcess(int process)
{
	struct ProcessMsg msg;
	msg.type = ProcessWait;
	Object_Send(process, &msg, sizeof(msg), NULL, 0);
}