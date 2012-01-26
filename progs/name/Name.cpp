#include <System.h>
#include <Object.h>
#include <Message.h>
#include <Kernel.h>

#include <stdlib.h>
#include <string.h>

#include <kernel/include/NameFmt.h>
#include <kernel/include/Syscalls.h>

#include <vector>

using std::vector;

struct NameEntry {
	char name[32];
	int object;
};

vector<NameEntry*> nameList;

struct NameEntry *findEntry(const char *name)
{
	for(int i=0; i<nameList.size(); i++) {
		if(!strcmp(name, nameList[i]->name)) {
			return nameList[i];
		}
	}

	return NULL;
}

int lookup(const char *name)
{
	struct NameEntry *entry = findEntry(name);

	if(entry == NULL) {
		return OBJECT_INVALID;
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
		nameList.push_back(entry);
	} else {
		entry->object = object;
	}
}

int main(int argc, char *argv[])
{
	int obj = Object_Create(OBJECT_INVALID, NULL);

	Kernel_SetObject(KernelObjectNameServer, obj);

	while(1) {
		union NameMsg msg;
		int m;

		m = Object_Receive(obj, &msg, sizeof(msg));

		if(m == 0) {
			continue;
		}

		switch(msg.msg.type) {
			case NameMsgTypeLookup:
			{
				int ret = lookup(msg.msg.u.lookup.name);
				struct BufferSegment segs[] = { &ret, sizeof(ret) };
				struct MessageHeader hdr = { segs, 1, 0, 1 };
				Message_Replyx(m, 0, &hdr);
				break;
			}

			case NameMsgTypeSet:
			{
				set(msg.msg.u.set.name, msg.msg.u.set.obj);
				Message_Reply(m, 0, NULL, 0);
				break;
			}
		}
	}
}