#include <System.h>
#include <Message.h>

#include "Internal.h"

#include <kernel/include/MessageFmt.h>
#include <kernel/include/ProcManagerFmt.h>

#include <string.h>
#include <stddef.h>

int SpawnProcess(const char *argv[], int stdinObject, int stdoutObject, int stderrObject)
{
	struct MessageHeader hdr;
	union ProcManagerMsg msg;
	struct BufferSegment segs[] = { &msg, sizeof(msg) };
	int child;
	int i;
	char *c;

	msg.msg.type = ProcManagerSpawnProcess;

	// Construct the command line out of the passed-in argv
	c = msg.msg.u.spawn.cmdline;
	for(i=0; argv[i] != NULL; i++) {
		strcpy(c, argv[i]);
		c += strlen(argv[i]) + 1;
	}
	*c = '\0';

	msg.msg.u.spawn.stdinObject = stdinObject;
	msg.msg.u.spawn.stdoutObject = stdoutObject;
	msg.msg.u.spawn.stderrObject = stderrObject;

	hdr.segments = segs;
	hdr.numSegments = 1;
	hdr.objectsOffset = offsetof(union ProcManagerMsg, msg.u.spawn.stdinObject);
	hdr.objectsSize = 3;

	Object_Sendxs(PROCMAN_NO, &hdr, &child, sizeof(child));
	return child;
}

void WaitProcess(int process)
{
	union ProcManagerMsg msg;
	msg.msg.type = ProcManagerWait;
	Object_Send(process, &msg, sizeof(msg), NULL, 0);
}