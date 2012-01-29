#include <System.h>
#include <Object.h>
#include <Message.h>
#include <Kernel.h>

#include <stdlib.h>
#include <string.h>

#include <kernel/include/NameFmt.h>
#include <kernel/include/Syscalls.h>

#include <vector>
#include <string>

using std::vector;
using std::string;

struct NameEntry {
	string name;
	vector<int> objects;
	vector<NameEntry*> children;
	vector<int> waiters;
};

NameEntry root;

vector<string> splitPath(const string &path)
{
	vector<string> ret;
	int i, j;

	i=1;
	for(j=1; j<path.size(); j++) {
		if(path[j] == '/') {
			ret.push_back(path.substr(i, j-i));
			i = j + 1;
		}
	}

	ret.push_back(path.substr(i));

	return ret;
}

vector<int> lookup(string name)
{
	vector<string> segs = splitPath(name);
	vector<int> ret;
	NameEntry *cursor = &root;

	ret.insert(ret.end(), root.objects.begin(), root.objects.end());

	for(int i=0; i<segs.size(); i++) {
		NameEntry *child = NULL;
		for(int j=0; j<cursor->children.size(); j++) {
			if(cursor->children[j]->name == segs[i]) {
				child = cursor->children[j];
				break;
			}
		}

		if(child == NULL) {
			break;
		} else {
			cursor = child;
			ret.insert(ret.end(), cursor->objects.begin(), cursor->objects.end());
		}
	}

	return ret;
}

void addWaiter(string name, int message)
{
	vector<string> segs = splitPath(name);
	NameEntry *cursor = &root;

	for(int i=0; i<segs.size(); i++) {
		NameEntry *child = NULL;
		for(int j=0; j<cursor->children.size(); j++) {
			if(cursor->children[j]->name == segs[i]) {
				child = cursor->children[j];
				break;
			}
		}

		if(child == NULL) {
			child = new NameEntry;

			child->name = segs[i];
			cursor->children.push_back(child);
		}

		cursor = child;
	}

	cursor->waiters.push_back(message);
}

void set(const string &name, int object)
{
	vector<string> segs = splitPath(name);
	NameEntry *cursor = &root;

	for(int i=0; i<segs.size(); i++) {
		NameEntry *child = NULL;
		for(int j=0; j<cursor->children.size(); j++) {
			if(cursor->children[j]->name == segs[i]) {
				child = cursor->children[j];
				break;
			}
		}

		if(child == NULL) {
			child = new NameEntry;

			child->name = segs[i];
			cursor->children.push_back(child);
		}

		cursor = child;
	}

	cursor->objects.push_back(object);

	for(int i=0; i<cursor->waiters.size(); i++) {
		Message_Reply(cursor->waiters[i], 0, NULL, 0);
	}
	cursor->waiters.clear();
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
				vector<int> objs = lookup(msg.msg.u.open.name);
				for(int i=objs.size() - 1; i>=0; i--) {
					Object_Send(objs[i], &msg, sizeof(union NameMsg), &ret, sizeof(ret));
					if(ret != OBJECT_INVALID) {
						break;
					}
				}
				Message_Replyx(m, 0, &hdr);
				Object_Release(ret);
				break;
			}

			case NameMsgTypeWait:
			{
				vector<int> objs = lookup(msg.msg.u.wait.name);
				if(objs.size() == 0) {
					addWaiter(msg.msg.u.wait.name, m);
				} else {
					Message_Reply(m, 0, NULL, 0);
				}
				break;
			}
		}
	}
}