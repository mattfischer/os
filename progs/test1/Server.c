#include <lib/system/Shared.h>

#include "Msg.h"

void _start()
{
	int obj = CreateObject();

	SetName("test", obj);

	while(1) {
		struct MessageHeader header;
		struct Msg msg;
		int m;

		header.size = sizeof(msg);
		header.body = &msg;

		m = ReceiveMessage(obj, &header);
		msg.x++;
		ReplyMessage(m, &header);
	}
}