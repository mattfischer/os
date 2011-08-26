#include <System.h>
#include <Message.h>

#include "Internal.h"

#include <kernel/include/MessageFmt.h>
#include <kernel/include/ProcManagerFmt.h>

#include <string.h>
#include <stddef.h>

void SpawnProcess(const char *name, int stdinObject, int stdoutObject, int stderrObject)
{
	struct MessageHeader hdr;
	union ProcManagerMsg msg;
	struct BufferSegment segs[] = { &msg, sizeof(msg) };

	msg.msg.type = ProcManagerSpawnProcess;
	strcpy(msg.msg.u.spawn.name, name);
	msg.msg.u.spawn.stdinObject = stdinObject;
	msg.msg.u.spawn.stdoutObject = stdoutObject;
	msg.msg.u.spawn.stderrObject = stderrObject;

	hdr.segments = segs;
	hdr.numSegments = 1;
	hdr.objectsOffset = offsetof(union ProcManagerMsg, msg.u.spawn.stdinObject);
	hdr.objectsSize = 3;

	Object_Sendxs(__ProcessManager, &hdr, NULL, 0);
}