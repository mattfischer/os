#include <Object.h>
#include <Message.h>
#include <System.h>
#include <Name.h>
#include <Channel.h>

#include <stddef.h>
#include <stdio.h>

#include <kernel/include/IOFmt.h>
#include <kernel/include/NameFmt.h>

#include <algorithm>
#include <list>

volatile unsigned *uartbase;

#define UARTDR   (uartbase + 0)
#define UARTIMSC (uartbase + 14)
#define UARTMIS  (uartbase + 16)
#define UARTICR  (uartbase + 17)

struct Waiter {
	int m;
	int size;
};

#define BUFFER_SIZE 4096
char buffer[BUFFER_SIZE];
int readPointer = 0;
int writePointer = 0;

struct Info {
};

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
	int channel = Channel_Create();
	int server = Object_Create(channel, 0);

	Name_Set(argv[1], server);

	sscanf(argv[2], "0x%x", &uartbase);
	MapPhys((void*)uartbase, (int)uartbase, 4096);
	*UARTIMSC = 0x10;
	sub = Interrupt_Subscribe(1, server, IRQEvent, 0);

	while(1) {
		union {
			union IOMsg io;
			union NameMsg name;
		} msg;
		unsigned targetData;
		int m;

		m = Channel_Receive(channel, &msg, sizeof(msg), &targetData);

		if(m == 0) {
			switch(msg.name.event.type) {
				case SysEventObjectClosed:
				{
					struct Info *info = (struct Info*)msg.name.event.targetData;
					delete info;
					break;
				}

				case IRQEvent:
				{
					unsigned status = *UARTMIS;
					if(status & 0x10) {
						char input = (char)*UARTDR;
						if(writePointer != readPointer - 1) {
							buffer[writePointer] = input;
							writePointer++;
							writePointer %= BUFFER_SIZE;
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

		if(targetData == 0) {
			switch(msg.name.msg.type) {
				case NameMsgTypeOpen:
				{
					int obj;
					struct Info *info = new Info;
					obj = Object_Create(channel, (unsigned)info);
					Message_Replyh(m, 0, &obj, sizeof(obj), 0, 1);
					Object_Release(obj);
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