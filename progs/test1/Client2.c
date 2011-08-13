#include <Object.h>
#include <Message.h>
#include <Name.h>

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

		strcpy(msg.message, "B\r\n");
		SendMessage(obj, &hdr, NULL);
	}
}