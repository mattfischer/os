#include <lib/system/Object.h>
#include <lib/system/Message.h>
#include <lib/system/Name.h>

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