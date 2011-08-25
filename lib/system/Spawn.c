#include "include/System.h"
#include "include/Message.h"

#include "Internal.h"

#include <kernel/include/MessageFmt.h>
#include <kernel/include/ProcManagerFmt.h>

#include <string.h>
#include <stddef.h>

void SpawnProcess(const char *name, int stdinObject, int stdoutObject, int stderrObject)
{
	struct MessageHeader hdr;
	struct ProcManagerMsg msg;
	struct BufferSegment segs[] = { &msg, sizeof(msg) };

	msg.type = ProcManagerSpawnProcess;
	strcpy(msg.u.spawn.name, name);
	msg.u.spawn.stdinObject = stdinObject;
	msg.u.spawn.stdoutObject = stdoutObject;
	msg.u.spawn.stderrObject = stderrObject;

	hdr.segments = segs;
	hdr.numSegments = 1;
	hdr.objectsOffset = offsetof(struct ProcManagerMsg, u.spawn.stdinObject);
	hdr.objectsSize = 3;

	Object_Sendxs(__ProcessManager, &hdr, NULL, 0);
}