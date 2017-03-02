#include <stdio.h>

#include <kernel/include/ProcManagerFmt.h>
#include <Object.h>
#include <lib/system/Internal.h>

#define BUFFER_SIZE 128

void main()
{
	char buffer[BUFFER_SIZE];
	union ProcManagerMsg msg;
	int offset = 0;
	int size = 0;

	do {
		msg.msg.type = ProcManagerReadLog;
		msg.msg.u.readLog.offset = offset;
		msg.msg.u.readLog.size = BUFFER_SIZE - 1;

		size = Object_Send(PROCMAN_NO, &msg, sizeof(msg), buffer, BUFFER_SIZE);
		buffer[size] = '\0';
		offset += size;
		printf(buffer);
	} while(size > 0);
}