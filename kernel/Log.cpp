#include "Log.hpp"

#include "Task.hpp"
#include "Kernel.hpp"
#include "Process.hpp"
#include "Slab.hpp"

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
bool full = false;
int logServer;

struct Info {
	int obj;
	int pointer;
};

static Slab<Info> infoSlab;

void Log::puts(const char *s)
{
	int size = strlen(s);
	int len = std::min(size, LOG_BUFFER_SIZE - writePointer);
	memcpy(buffer + writePointer, s, len);
	writePointer += len;
	if(writePointer == LOG_BUFFER_SIZE) {
		writePointer = 0;
		full = true;
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
			switch(msg.name.event.type) {
				case SysEventObjectClosed:
				{
					Info *info = (Info*)msg.name.event.targetData;
					if(info) {
						Object_Release(info->obj);
						infoSlab.free(info);
					}
				}
			}
			continue;
		}

		MessageInfo messageInfo;
		Message_Info(m, &messageInfo);
		Info *info = (Info*)messageInfo.targetData;

		if(info == NULL) {
			switch(msg.name.msg.type) {
				case NameMsgTypeOpen:
				{
					int obj;
					info = infoSlab.allocate();
					obj = Object_Create(logServer, info);

					info->obj = obj;
					info->pointer = 0;

					struct BufferSegment segs[] = { &obj, sizeof(obj) };
					struct MessageHeader hdr = { segs, 1, 0, 1 };

					Message_Replyx(m, 0, &hdr);
					break;
				}
			}
		} else {
			switch(msg.name.msg.type) {
				case IOMsgTypeRead:
				{
					int len;
					int start;

					if(full) {
						start = writePointer;
						len = LOG_BUFFER_SIZE;
					} else {
						start = 0;
						len = writePointer;
					}

					start += info->pointer;
					start %= LOG_BUFFER_SIZE;

					int size = std::min(msg.io.msg.u.rw.size, len - start);
					size = std::min(size, LOG_BUFFER_SIZE - start);

					Message_Reply(m, size, (char*)buffer + start, size);
					info->pointer += size;
					break;
				}
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
