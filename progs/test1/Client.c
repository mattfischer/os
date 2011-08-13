#include <lib/system/Object.h>
#include <lib/system/Message.h>
#include <lib/system/Name.h>

#include <string.h>

#include "Msg.h"

int main(int argc, char *argv[])
{
	int obj = LookupName("test");

	while(1) {
		struct MessageHeader hdr;
		struct PrintMsg msg;

		hdr.size = sizeof(msg);
		hdr.body = &msg;

		strcpy(msg.message, "A\r\n");
		SendMessage(obj, &hdr, NULL);
	}
}