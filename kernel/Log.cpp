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
#include <stdarg.h>

#define LOG_BUFFER_SIZE 4096
char buffer[LOG_BUFFER_SIZE];
int writePointer = 0;
bool full = false;
int logServer;
int logChannel;

struct Info {
	int obj;
	int pointer;
};

static Slab<Info> infoSlab;

static void printHex(unsigned int x, bool fill)
{
	char buffer[9];
	int n = 0;

	for(int i=0; i<8; i++) {
		int f = (x >> (28 - i * 4)) & 0xf;
		if(fill || f != 0) {
			if(f < 10) {
				buffer[n] = '0' + f;
			} else {
				buffer[n] = 'a' + f - 10;
			}
			n++;
		}
	}
	buffer[n] = '\0';

	Log::puts(buffer);
}

static void printInt(unsigned int x)
{
	char buffer[16];
	int pow;

	for(pow = 1; pow * 10 < x; pow *= 10) ;

	int n = 0;
	for(; pow >= 1; pow /= 10) {
		buffer[n] = '0' + x / pow;
		n++;
		x -= pow * (x / pow);
	}
	buffer[n] = '\0';

	Log::puts(buffer);
}

void Log::printf(const char *s, ...)
{
	const char *start = s;
	const char *end;
	va_list va;

	va_start(va, s);
	while(*start != '\0') {
		end = start;
		while(*end != '\0' && *end != '%') end++;
		write(start, end - start);
		start = end;

		if(*start == '%') {
			start++;
			switch(*start) {
				case 's':
				{
					const char *s2 = va_arg(va, const char *);
					puts(s2);
					break;
				}

				case 'i':
				{
					unsigned int x = va_arg(va, int);
					printInt(x);
					break;
				}

				case 'x':
				{
					unsigned int x = va_arg(va, int);
					printHex(x, false);
					break;
				}

				case 'p':
				{
					unsigned int x = va_arg(va, int);
					puts("0x");
					printHex(x, true);
					break;
				}
			}
			start++;
		}
	}

	va_end(va);
}

void Log::puts(const char *s)
{
	int size = strlen(s);
	write(s, size);
}

void Log::write(const char *s, int size)
{
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
		int m = Channel_Receive(logChannel, &msg, sizeof(msg));

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

		if(info == 0) {
			switch(msg.name.msg.type) {
				case NameMsgTypeOpen:
				{
					int obj;
					info = infoSlab.allocate();
					obj = Object_Create(logChannel, info);

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
	logChannel = Channel_Create();
	logServer = Object_Create(logChannel, 0);
	Name_Set("/dev/log", logServer);

	Task *task = Kernel::process()->newTask();
	task->start(server, 0);
}
