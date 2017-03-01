#include "InitFs.hpp"

#include "Kernel.hpp"
#include "Object.hpp"
#include "Message.hpp"
#include "Sched.hpp"
#include "Task.hpp"
#include "Process.hpp"
#include "Log.hpp"

#include <kernel/include/InitFsFmt.h>
#include <kernel/include/NameFmt.h>
#include <kernel/include/IOFmt.h>

#include <lib/shared/include/Name.h>
#include <lib/shared/include/Kernel.h>

#include <algorithm>

#include <string.h>

// These symbols are populated by the linker script, and point
// to the beginning and end of the initfs data
extern char __InitFsStart[];
extern char __InitFsEnd[];

int fileServer;
int fileChannel;
Object *InitFs::sNameServer;

// Path at which to register the InitFS
#define PREFIX "/boot"

enum {
	RegisterEvent = SysEventLast
};

// List of registered names, to pass along to real name server
struct RegisteredName : public ListEntry {
	char name[32];
	int obj;
};

Slab<RegisteredName> registeredNameSlab;
List<RegisteredName> registeredNames;

// List of waiters
struct Waiter : public ListEntry {
	int message;
};

Slab<Waiter> waiterSlab;
List<Waiter> waiters;

static void *lookup(const char *name, int *size)
{
	// Search through the initfs searching for a matching file
	struct InitFsFileHeader *header = (struct InitFsFileHeader*)__InitFsStart;
	while((void*)header < (void*)__InitFsEnd) {
		if(!strcmp(header->name, name)) {
			// Filename matches
			if(size) {
				*size = header->size;
			}
			return (char*)header + sizeof(struct InitFsFileHeader);
		}

		// Skip to the next file record
		header = (struct InitFsFileHeader*)((char*)header + sizeof(struct InitFsFileHeader) + header->size);
	}

	return 0;
}

// Open file information structure
struct FileInfo {
	void *data;
	int size;
	int pointer;
};

struct DirInfo {
	struct InitFsFileHeader *header;
};

enum InfoType {
	InfoTypeFile,
	InfoTypeDir
};

struct Info {
	InfoType type;
	union {
		FileInfo file;
		DirInfo dir;
	} u;
};

static Slab<Info> infoSlab;

// InitFS file server.  This also implements a basic proto-name server,
// which is used until the real userspace name server is started.
static void server(void *param)
{
	Log::printf("initf: Starting server\n");

	while(1) {
		union {
			union NameMsg name;
			union IOMsg io;
		} msg;
		int m = Channel_Receive(fileChannel, &msg, sizeof(msg));

		if(m == 0) {
			switch(msg.name.event.type) {
				case SysEventObjectClosed:
				{
					Info *info = (Info*)msg.name.event.targetData;
					if(info) {
						infoSlab.free(info);
					}
					break;
				}

				case RegisterEvent:
				{
					Log::printf("initfs: Handing off to userspace server\n");

					// The new name server has been started.  Connect this file server
					// into it.
					Name_Set(PREFIX, fileServer);

					// Forward along any registered names to the new name server
					for(RegisteredName *registeredName = registeredNames.head(); registeredName != 0; registeredName = registeredNames.next(registeredName)) {
						Name_Set(registeredName->name, registeredName->obj);
						Object_Release(registeredName->obj);
					}

					// Fail any waiters so that they will retry the wait with the new name server
					for(Waiter *waiter = waiters.head(); waiter != 0; waiter = waiters.next(waiter)) {
						Message_Reply(waiter->message, -1, 0, 0);
					}
					break;
				}
			}
			continue;
		}

		struct MessageInfo messageInfo;
		Message_Info(m, &messageInfo);

		if(messageInfo.targetData == 0) {
			// Call made to the main file server object
			switch(msg.name.msg.type) {
				case NameMsgTypeOpen:
				{
					int size;
					void *data;
					int obj = OBJECT_INVALID;
					char *name = msg.name.msg.u.open.name;

					if(strncmp(name, PREFIX, strlen(PREFIX)) == 0) {
						Log::printf("initfs: Open file %s\n", name);

						// Look up the requested file name in the InitFS
						name += strlen(PREFIX) + 1;
						data = lookup(name, &size);
						if(data) {
							// File was found.  Create an object for the new
							// file and return it
							Info *info = infoSlab.allocate();
							info->type = InfoTypeFile;
							info->u.file.data = data;
							info->u.file.size = size;
							info->u.file.pointer = 0;
							obj = Object_Create(fileChannel, info);
						}
					}

					Message_Replyh(m, 0, &obj, sizeof(obj), 0, 1);
					if(obj != OBJECT_INVALID) {
						Object_Release(obj);
					}

					break;
				}

				case NameMsgTypeSet:
				{
					// This proto-name server will not actually respond to arbitrary
					// name sets, but it wil record them so that it can pass them along
					// to the real name server once it's registered.
					RegisteredName *registeredName = registeredNameSlab.allocate();
					strcpy(registeredName->name, msg.name.msg.u.set.name);
					registeredName->obj = msg.name.msg.u.set.obj;
					registeredNames.addTail(registeredName);
					Message_Reply(m, -1, 0, 0);
					break;
				}

				case NameMsgTypeWait:
				{
					// Record all waiters, so that we can fail them and restart them
					// against the new name server
					Waiter *waiter = waiterSlab.allocate();
					waiter->message = m;
					waiters.addTail(waiter);
					break;
				}

				case NameMsgTypeOpenDir:
				{
					int size;
					void *data;
					int obj = OBJECT_INVALID;
					char *name = msg.name.msg.u.open.name;

					if(strcmp(name, PREFIX) == 0) {
						Info *info = infoSlab.allocate();
						info->type = InfoTypeDir;
						info->u.dir.header = (struct InitFsFileHeader*)__InitFsStart;
						obj = Object_Create(fileChannel, info);
					}

					Message_Replyh(m, 0, &obj, sizeof(obj), 0, 1);
					if(obj != OBJECT_INVALID) {
						Object_Release(obj);
					}
					break;
				}
			}
		} else {
			Info *info = (Info*)messageInfo.targetData;

			// Message was sent to an open file handle
			switch(msg.io.msg.type) {
				case IOMsgTypeRead:
				{
					int size = std::min(msg.io.msg.u.rw.size, info->u.file.size - info->u.file.pointer);
					Message_Reply(m, size, (char*)info->u.file.data + info->u.file.pointer, size);
					break;
				}

				case IOMsgTypeSeek:
				{
					info->u.file.pointer = msg.io.msg.u.seek.pointer;
					Message_Reply(m, 0, 0, 0);
					break;
				}

				case IOMsgTypeReadDir:
				{
					IOMsgReadDirRet ret;
					int status = 1;
					if((void*)info->u.dir.header < (void*)__InitFsEnd){
						status = 0;
						strcpy(ret.name, info->u.dir.header->name);
						info->u.dir.header = (struct InitFsFileHeader*)((char*)info->u.dir.header + sizeof(struct InitFsFileHeader) + info->u.dir.header->size);
					}
					Message_Reply(m, status, &ret, sizeof(ret));
					break;
				}
			}
		}
	}
}

/*!
 * \brief Start the InitFS server
 */
void InitFs::start()
{
	fileChannel = Channel_Create();
	fileServer = Object_Create(fileChannel, 0);
	sNameServer = Sched::current()->process()->object(fileServer);

	Task *task = Kernel::process()->newTask();
	task->start(server, 0);
}

/*!
 * \brief Set the name server object
 * \param nameServer New name server
 */
void InitFs::setNameServer(Object *nameServer)
{
	sNameServer = nameServer;

	// Inform the proto-name server that a real name server has been registered
	Kernel::process()->object(fileServer)->post(RegisterEvent, 0);
}

/*!
 * \brief Retrieve the name server object
 * \return Name server
 */
Object* InitFs::nameServer()
{
	return sNameServer;
}
