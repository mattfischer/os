#include <Object.h>
#include <Message.h>
#include <System.h>

#include <stddef.h>

#include <kernel/include/IOFmt.h>
#include <kernel/include/NameFmt.h>

#define UARTBASE (volatile unsigned*)0x16000000
#define UARTDR   (UARTBASE + 0)
#define UARTIMSC (UARTBASE + 14)
#define UARTMIS  (UARTBASE + 16)
#define UARTICR  (UARTBASE + 17)

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

enum {
	IRQEvent = SysEventLast
};

int main(int argc, char *argv[])
{
	int sub;
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
						*UARTDR = input;
					}

					Interrupt_Acknowledge(1, sub);
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
			}
		}
	}
}