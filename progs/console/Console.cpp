#include <Object.h>
#include <Message.h>
#include <System.h>
#include <Name.h>

#include <stddef.h>

#include <kernel/include/IOFmt.h>
#include <kernel/include/NameFmt.h>

#include <algorithm>
#include <list>

#define UARTBASE (volatile unsigned*)0x16000000
#define UARTDR   (UARTBASE + 0)
#define UARTIMSC (UARTBASE + 14)
#define UARTMIS  (UARTBASE + 16)
#define UARTICR  (UARTBASE + 17)

struct Waiter {
	int m;
	int size;
};

#define BUFFER_SIZE 4096
char buffer[BUFFER_SIZE];
int readPointer = 0;
int writePointer = 0;


void PrintUart(char *buffer, int size)
{
	int i;
	volatile unsigned *out = UARTDR;

	for(i=0; i<size; i++) {
		if(buffer[i] == '\n') {
			*out = '\r';
		}

		*out = buffer[i];
	}
}

void returnData(int m, int size)
{
	int retSize;
	if(writePointer > readPointer) {
		retSize = std::min(writePointer - readPointer, size);
	} else {
		retSize = std::min(BUFFER_SIZE - readPointer, size);
	}
	Message_Reply(m, retSize, buffer + readPointer, retSize);
	readPointer += retSize;
	readPointer %= BUFFER_SIZE;
}

enum {
	IRQEvent = SysEventLast
};

int main(int argc, char *argv[])
{
	int sub;
	std::list<Waiter> waiters;
	int obj = Object_Create(OBJECT_INVALID, NULL);

	Name_Set("/dev/console", obj);

	MapPhys((void*)UARTBASE, 0x16000000, 4096);
	*UARTIMSC = 0x10;
	sub = Interrupt_Subscribe(1, obj, IRQEvent, 0);

	while(1) {
		union {
			union IOMsg io;
			union NameMsg name;
		} msg;
		struct MessageInfo info;
		int m;

		m = Object_Receive(obj, &msg, sizeof(msg));

		if(m == 0) {
			switch(msg.name.event.type) {
				case IRQEvent:
				{
					unsigned status = *UARTMIS;
					if(status & 0x10) {
						char input = (char)*UARTDR;
						if(writePointer != readPointer - 1) {
							if(input == '\r') {
								input = '\n';
							}
							buffer[writePointer] = input;
							writePointer++;
							writePointer %= BUFFER_SIZE;
							if(input != '\n') {
								*UARTDR = input;
							}
						}

						if(!waiters.empty()) {
							Waiter w = waiters.front();
							waiters.pop_front();
							returnData(w.m, w.size);
						}
					}

					Interrupt_Acknowledge(sub);
					break;
				}
			}
			continue;
		}

		Message_Info(m, &info);

		if(info.targetData == NULL) {
			switch(msg.name.msg.type) {
				case NameMsgTypeOpen:
				{
					int child;
					struct BufferSegment segs[] = { &child, sizeof(child) };
					struct MessageHeader hdr = { segs, 1, 0, 1 };

					child = Object_Create(obj, (void*)0x1234);

					Message_Replyx(m, 0, &hdr);
					break;
				}
			}
		} else {
			switch(msg.io.msg.type) {
				case IOMsgTypeWrite:
				{
					char buffer[256];
					int sent;
					int headerSize;

					headerSize = offsetof(union IOMsg, msg.u.rw) + sizeof(msg.io.msg.u.rw);
					sent = 0;
					while(sent < msg.io.msg.u.rw.size) {
						int size;

						size = Message_Read(m, buffer, headerSize + sent, sizeof(buffer));
						PrintUart(buffer, size);
						sent += size;
					}
					Message_Reply(m, sent, NULL, 0);
					break;
				}

				case IOMsgTypeRead:
				{
					if(readPointer == writePointer) {
						struct Waiter w;
						w.m = m;
						w.size = msg.io.msg.u.rw.size;
						waiters.push_back(w);
					} else {
						returnData(m, msg.io.msg.u.rw.size);
					}
				}
			}
		}
	}
}