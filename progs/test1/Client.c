#include <Object.h>
#include <Message.h>
#include <Name.h>

#include <string.h>
#include <stdlib.h>

#include "Msg.h"

int main(int argc, char *argv[])
{
	int obj = LookupName("test");
	while(1) {
		struct PrintMsg msg;

		strcpy(msg.message, "A\r\n");
		SendMessage(obj, &msg, sizeof(msg), NULL, 0);
	}
}