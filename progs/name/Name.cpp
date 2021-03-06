#include <System.h>
#include <Object.h>
#include <Message.h>
#include <Channel.h>

#include <stdlib.h>
#include <string.h>

#include <kernel/include/NameFmt.h>
#include <kernel/include/IOFmt.h>
#include <kernel/include/Syscalls.h>
#include <kernel/include/Objects.h>

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

struct OpenDir {
	struct NameEntry *entry;
	int obj;
	int entryIdx;
	vector<int> objects;
	int objectIdx;
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

	if(i < path.size()) {
		ret.push_back(path.substr(i));
	}

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

struct OpenDir *createOpenDir(const char *name)
{
	vector<string> segs = splitPath(name);
	NameEntry *cursor = &root;

	for(int i=0; i<segs.size(); i++) {
		bool found = false;
		for(int j=0; j<cursor->children.size(); j++) {
			if(cursor->children[j]->name == segs[i]) {
				cursor = cursor->children[j];
				found = true;
				break;
			}
		}

		if(!found) {
			cursor = NULL;
			break;
		}
	}

	vector<int> servers = lookup(name);
	vector<int> objs;
	for(int i=0; i<servers.size(); i++) {
		int obj;
		struct NameMsg msg;
		msg.type = NameMsgTypeOpenDir;
		strcpy(msg.openDir.name, name);
		Object_Send(servers[i], &msg, sizeof(msg), &obj, sizeof(obj));
		if(obj != OBJECT_INVALID) {
			objs.push_back(obj);
		}
	}

	struct OpenDir *openDir = NULL;
	if(cursor || objs.size() > 0) {
		openDir = new OpenDir;
		openDir->entry = cursor;
		openDir->entryIdx = 0;
		openDir->objects = objs;
		openDir->objectIdx = 0;
	}

	return openDir;
}

int main(int argc, char *argv[])
{
	int channel = Channel_Create();
	int obj = Object_Create(channel, 0);

	set("/boot", NAMESERVER_NO);

	int child;
	const char *childArgv[4];

	childArgv[0] = argv[1];
	childArgv[1] = NULL;
	child = SpawnProcessx(childArgv, OBJECT_INVALID, OBJECT_INVALID, OBJECT_INVALID, obj);
	Object_Release(child);

	while(1) {
		struct NameMsg msg;
		int m;
		unsigned targetData;

		m = Channel_Receive(channel, &msg, sizeof(msg), &targetData);

		if(m == 0) {
			switch(msg.event.type) {
				case SysEventObjectClosed:
				{
					struct OpenDir *openDir = (struct OpenDir*)targetData;
					if(openDir) {
						for(int i = 0; i < openDir->objects.size(); i++) {
							Object_Release(openDir->objects[i]);
						}
						delete openDir;
					}
					break;
				}
			}
			continue;
		}

		if(targetData == 0) {
			switch(msg.type) {
				case NameMsgTypeSet:
				{
					set(msg.set.name, msg.set.obj);
					Message_Reply(m, 0, NULL, 0);
					break;
				}

				case NameMsgTypeOpen:
				{
					int ret = OBJECT_INVALID;
					vector<int> objs = lookup(msg.open.name);
					for(int i=objs.size() - 1; i>=0; i--) {
						Object_Send(objs[i], &msg, sizeof(struct NameMsg), &ret, sizeof(ret));
						if(ret != OBJECT_INVALID) {
							break;
						}
					}
					Message_Replyh(m, 0, &ret, sizeof(ret), 0, 1);
					Object_Release(ret);
					break;
				}

				case NameMsgTypeOpenDir:
				{
					struct OpenDir *openDir = createOpenDir(msg.openDir.name);
					int ret = OBJECT_INVALID;
					if(openDir) {
						ret = Object_Create(channel, (unsigned)openDir);
					}
					Message_Replyh(m, 0, &ret, sizeof(ret), 0, 1);
					Object_Release(ret);
					break;
				}

				case NameMsgTypeWait:
				{
					vector<int> objs = lookup(msg.wait.name);
					if(objs.size() == 0) {
						addWaiter(msg.wait.name, m);
					} else {
						Message_Reply(m, 0, NULL, 0);
					}
					break;
				}
			}
		} else {
			struct OpenDir *openDir = (struct OpenDir*)targetData;
			switch(msg.type) {
				case IOMsgTypeReadDir:
				{
					struct IOMsgReadDirRet ret;
					int status = 1;
					if(openDir->entry && openDir->entryIdx < openDir->entry->children.size()) {
						string &name = openDir->entry->children[openDir->entryIdx]->name;
						strcpy(ret.name, name.c_str());
						status = 0;
						openDir->entryIdx++;
					} else {
						while(openDir->objectIdx < openDir->objects.size()) {
							status = Object_Send(openDir->objects[openDir->objectIdx], &msg, sizeof(msg), &ret, sizeof(ret));

							if(status == 1) {
								openDir->objectIdx++;
							} else {
								break;
							}
						}
					}
					Message_Reply(m, status, &ret, sizeof(ret));
					break;
				}
			}
		}
	}
}