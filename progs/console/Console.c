#include <Object.h>
#include <Message.h>
#include <System.h>

#include <stddef.h>

#include <kernel/include/IOFmt.h>
#include <kernel/include/NameFmt.h>

void PrintUart(char *uart, char *buffer, int size)
{
	int i;
	volatile char *out = uart;

	for(i=0; i<size; i++) {
		if(buffer[i] == '\n') {
			*out = '\r';
		}

		*out = buffer[i];
	}
}

int main(int argc, char *argv[])
{
	char *uart = (char*)0x16000000;
	int obj = Object_Create(OBJECT_INVALID, NULL);

	Name_Set("/dev/console", obj);

	MapPhys(uart, 0x16000000, 4096);
	while(1) {
		union {
			union IOMsg io;
			union NameMsg name;
		} msg;
		struct MessageInfo info;
		int m;

		m = Object_Receive(obj, &msg, sizeof(msg));

		if(m == 0) {
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
						PrintUart(uart, buffer, size);
						sent += size;
					}
					Message_Reply(m, sent, NULL, 0);
					break;
				}
			}
		}
	}
}