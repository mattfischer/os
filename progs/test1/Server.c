#include <Object.h>
#include <Message.h>
#include <Name.h>
#include <Map.h>

#include <malloc.h>
#include <stddef.h>

#include <kernel/include/IOFmt.h>

void PrintUart(char *uart, char *message)
{
	while(*message != '\0') {
		if(*message == '\n') {
			*uart = '\r';
		}

		*uart = *message;
		message++;
	}
}

int main(int argc, char *argv[])
{
	char *uart = (char*)0x16000000;
	int obj = CreateObject();

	SetName("test", obj);

	MapPhys(uart, 0x16000000, 4096);
	while(1) {
		struct IOMsg msg;
		int m;

		m = ReceiveMessage(obj, &msg, sizeof(msg));
		switch(msg.type) {
			case IOMsgTypeWrite:
			{
				char *buffer = malloc(msg.u.write.size);
				ReadMessage(m, buffer, offsetof(struct IOMsg, u.write.data), msg.u.write.size);
				PrintUart(uart, buffer);
				free(buffer);
				ReplyMessage(m, 0, NULL, 0);
				break;
			}
		}
	}
}