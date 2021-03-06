#include <stdio.h>

#include <kernel/include/KernelFmt.h>
#include <kernel/include/Objects.h>
#include <Object.h>

#define BUFFER_SIZE 128

void main()
{
	char buffer[BUFFER_SIZE];
	struct KernelMsg msg;
	int offset = 0;
	int size = 0;

	do {
		msg.type = KernelReadLog;
		msg.readLog.offset = offset;
		msg.readLog.size = BUFFER_SIZE - 1;

		size = Object_Send(KERNEL_NO, &msg, sizeof(msg), buffer, BUFFER_SIZE);
		buffer[size] = '\0';
		offset += size;
		printf(buffer);
	} while(size > 0);
}