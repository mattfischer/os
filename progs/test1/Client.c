#include "Shared.h"
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