#include <lib/system/Object.h>
#include <lib/system/Message.h>
#include <lib/system/Name.h>
#include <lib/system/Map.h>

#include <stddef.h>

#include "Msg.h"

void PrintUart(char *uart, char *message)
{
	while(*message != '\0') {
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
		struct MessageHeader header;
		struct PrintMsg msg;
		int m;

		header.size = sizeof(msg);
		header.body = &msg;

		m = ReceiveMessage(obj, &header);
		PrintUart(uart, msg.message);
		ReplyMessage(m, 0, NULL);
	}
}