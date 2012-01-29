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

struct NameWaiter {
	char name[32];
	int message;
};

vector<NameWaiter*> waiters;

struct NameEntry *findEntry(const char *name)
{
	for(int i=0; i<nameList.size(); i++) {
		char *listName = nameList[i]->name;
		if(!strncmp(name, listName, strlen(listName))) {
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

	for(int i=0; i<waiters.size(); i++) {
		if(strcmp(waiters[i]->name, name) == 0) {
			Message_Reply(waiters[i]->message, 0, NULL, 0);
			waiters.erase(waiters.begin() + i);
			i--;
		}
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

			case NameMsgTypeOpen:
			{
				int ret = OBJECT_INVALID;
				struct BufferSegment segs[] = { &ret, sizeof(ret) };
				struct MessageHeader hdr = { segs, 1, 0, 1 };
				int obj = lookup(msg.msg.u.open.name);
				if(obj != OBJECT_INVALID) {
					Object_Send(obj, &msg, sizeof(union NameMsg), &ret, sizeof(ret));
				}
				Message_Replyx(m, 0, &hdr);
				Object_Release(ret);
				break;
			}

			case NameMsgTypeWait:
			{
				int obj = lookup(msg.msg.u.wait.name);
				if(obj == OBJECT_INVALID) {
					NameWaiter *waiter = new NameWaiter;
					strcpy(waiter->name, msg.msg.u.wait.name);
					waiter->message = m;
					waiters.push_back(waiter);
				} else {
					Message_Reply(m, 0, NULL, 0);
				}
				break;
			}
		}
	}
}