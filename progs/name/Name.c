#include <System.h>
#include <Object.h>
#include <Message.h>

#include <stdlib.h>
#include <string.h>
#include <kernel/include/NameFmt.h>

struct NameEntry {
	char name[32];
	int object;
	struct NameEntry *next;
};

struct NameEntry *nameList = NULL;

struct NameEntry *findEntry(const char *name)
{
	struct NameEntry *entry;
	for(entry = nameList; entry != NULL; entry = entry->next) {
		if(!strcmp(name, entry->name)) {
			return entry;
		}
	}

	return NULL;
}

int lookup(const char *name)
{
	struct NameEntry *entry = findEntry(name);

	if(entry == NULL) {
		return INVALID_OBJECT;
	} else {
		return entry->object;
	}
}

void set(const char *name, int object)
{
	struct NameEntry *entry = findEntry(name);

	if(entry == NULL) {
		entry = (struct NameEntry*)malloc(sizeof(struct NameEntry));

		strcpy(entry->name, name);
		entry->object = object;
		entry->next = nameList;
		nameList = entry;
	} else {
		entry->object = object;
	}
}

int main(int argc, char *argv[])
{
	int obj = Object_Create();

	Kernel_SetObject(KernelObjectNameServer, obj);

	while(1) {
		struct NameMsg msg;
		int m;

		m = Object_Receive(obj, &msg, sizeof(msg));
		switch(msg.type) {
			case NameMsgTypeLookup:
			{
				int ret = lookup(msg.u.lookup.name);
				struct BufferSegment segs[] = { &ret, sizeof(ret) };
				struct MessageHeader hdr = { segs, 1, 0, 1 };
				Message_Replyx(m, 0, &hdr);
				break;
			}

			case NameMsgTypeSet:
			{
				set(msg.u.set.name, msg.u.set.obj);
				Message_Reply(m, 0, NULL, 0);
				break;
			}
		}
	}
}