#include <lib/system/Object.h>
#include <lib/system/Message.h>
#include <lib/system/Name.h>

#include "Msg.h"

void _start()
{
	int obj = LookupName("test");
	int x = 0;

	while(1) {
		struct MessageHeader hdr;
		struct Msg msg;

		hdr.size = sizeof(msg);
		hdr.body = &msg;

		msg.x = x;
		SendMessage(obj, &hdr, &hdr);

		x = msg.x;
	}
}