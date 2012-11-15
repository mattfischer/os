#include "Log.hpp"

#include "Task.hpp"
#include "Kernel.hpp"
#include "Process.hpp"

#include "include/NameFmt.h"
#include "include/IOFmt.h"

#include <lib/shared/include/Name.h>
#include <lib/shared/include/Object.h>
#include <lib/shared/include/Message.h>

#include <algorithm>
#include <string.h>

#define LOG_BUFFER_SIZE 4096
char buffer[LOG_BUFFER_SIZE];
int writePointer = 0;
int readPointer = 0;
int logServer;

void Log::puts(const char *s)
{
	int size = strlen(s);
	int len = std::min(size, LOG_BUFFER_SIZE - writePointer);
	memcpy(buffer + writePointer, s, len);
	writePointer += len;
	if(writePointer == LOG_BUFFER_SIZE) {
		writePointer = 0;
	}

	if(len < size) {
		memcpy(buffer, s + len, size - len);
		writePointer += size - len;
	}
}

static void server(void *param)
{
	while(1) {
		union {
				union NameMsg name;
				union IOMsg io;
			} msg;
		int m = Object_Receive(logServer, &msg, sizeof(msg));

		if(m == 0) {
			continue;
		}

		switch(msg.name.msg.type) {
			case NameMsgTypeOpen:
			{
				struct BufferSegment segs[] = { &logServer, sizeof(logServer) };
				struct MessageHeader hdr = { segs, 1, 0, 1 };

				Message_Replyx(m, 0, &hdr);
				break;
			}

			case IOMsgTypeRead:
			{
				int len;
				if(writePointer >= readPointer) {
					len = writePointer - readPointer;
				} else {
					len = LOG_BUFFER_SIZE - readPointer;
				}
				int size = std::min(msg.io.msg.u.rw.size, len);
				Message_Reply(m, size, (char*)buffer + readPointer, size);
				readPointer += size;
				if(readPointer == LOG_BUFFER_SIZE) {
					readPointer = 0;
				}
				break;
			}
		}
	}
}

void Log::start()
{
	logServer = Object_Create(OBJECT_INVALID, NULL);
	Name_Set("/dev/log", logServer);

	Task *task = Kernel::process()->newTask();
	task->start(server, NULL);
}
