#include <Object.h>
#include <Message.h>
#include <Name.h>
#include <Map.h>

#include <stddef.h>

#include <kernel/include/IOFmt.h>

void PrintUart(char *uart, char *buffer, int size)
{
	int i;

	for(i=0; i<size; i++) {
		if(buffer[i] == '\n') {
			*uart = '\r';
		}

		*uart = buffer[i];
	}
}

int main(int argc, char *argv[])
{
	char *uart = (char*)0x16000000;
	int obj = CreateObject();

	SetName("console", obj);

	MapPhys(uart, 0x16000000, 4096);
	while(1) {
		struct IOMsg msg;
		int m;

		m = ReceiveMessage(obj, &msg, sizeof(msg));
		switch(msg.type) {
			case IOMsgTypeWrite:
			{
				char buffer[256];
				int sent;
				int headerSize;

				headerSize = offsetof(struct IOMsg, u.write) + sizeof(msg.u.write);
				sent = 0;
				while(sent < msg.u.write.size) {
					int size;

					size = ReadMessage(m, buffer, headerSize + sent, sizeof(buffer));
					PrintUart(uart, buffer, size);
					sent += size;
				}
				ReplyMessage(m, 0, NULL, 0);
				break;
			}
		}
	}
}