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

#include <algorithm>

#include <string.h>

// These symbols are populated by the linker script, and point
// to the beginning and end of the initfs data
extern char __InitFsStart[];
extern char __InitFsEnd[];

// Path at which to register the InitFS
#define PREFIX "/boot"

static void *lookup(const char *name, int *size)
{
	// Search through the initfs searching for a matching file
	struct InitFsFileHeader *header = reinterpret_cast<struct InitFsFileHeader*>(__InitFsStart);
	while(reinterpret_cast<void*>(header) < reinterpret_cast<void*>(__InitFsEnd)) {
		if(!strcmp(header->name, name)) {
			// Filename matches
			if(size) {
				*size = header->size;
			}
			return reinterpret_cast<char*>(header) + sizeof(struct InitFsFileHeader);
		}

		// Skip to the next file record
		header = reinterpret_cast<struct InitFsFileHeader*>(reinterpret_cast<char*>(header) + sizeof(struct InitFsFileHeader) + header->size);
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
	};
};

static Slab<Info> infoSlab;

/*!
 * \brief Constructor
 */
InitFs::InitFs()
{
	mChannel = Channel_Create();
	mObject = Object_Create(mChannel, 0);
}

/*!
 * \brief Start the InitFS server
 */
void InitFs::start()
{
	Task *task = Kernel::process()->newTask();
	task->start(serverStatic, this);
}

/*!
 * \brief Retrieve the name server object
 * \return Name server
 */
int InitFs::object()
{
	return mObject;
}

void InitFs::serverStatic(void *param)
{
	InitFs *initfs = reinterpret_cast<InitFs*>(param);
	initfs->server();
}

// InitFS file server.  This also implements a basic proto-name server,
// which is used until the real userspace name server is started.
void InitFs::server()
{
	Log::printf("initf: Starting server\n");

	while(1) {
		union {
			struct NameMsg name;
			struct IOMsg io;
		} msg;
		unsigned targetData;
		int m = Channel_Receive(mChannel, &msg, sizeof(msg), &targetData);

		if(m == 0) {
			switch(msg.name.event.type) {
				case SysEventObjectClosed:
				{
					Info *info = reinterpret_cast<Info*>(targetData);
					if(info) {
						infoSlab.free(info);
					}
					break;
				}
			}
			continue;
		}

		if(targetData == 0) {
			// Call made to the main file server object
			switch(msg.name.type) {
				case NameMsgTypeOpen:
				{
					int size;
					void *data;
					int obj = OBJECT_INVALID;
					char *name = msg.name.open.name;

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
							info->file.data = data;
							info->file.size = size;
							info->file.pointer = 0;
							obj = Object_Create(mChannel, (unsigned)info);
						}
					}

					Message_Replyh(m, 0, &obj, sizeof(obj), 0, 1);
					if(obj != OBJECT_INVALID) {
						Object_Release(obj);
					}

					break;
				}

				case NameMsgTypeOpenDir:
				{
					int size;
					void *data;
					int obj = OBJECT_INVALID;
					char *name = msg.name.open.name;

					if(strcmp(name, PREFIX) == 0) {
						Info *info = infoSlab.allocate();
						info->type = InfoTypeDir;
						info->dir.header = reinterpret_cast<struct InitFsFileHeader*>(__InitFsStart);
						obj = Object_Create(mChannel, (unsigned)info);
					}

					Message_Replyh(m, 0, &obj, sizeof(obj), 0, 1);
					if(obj != OBJECT_INVALID) {
						Object_Release(obj);
					}
					break;
				}
			}
		} else {
			Info *info = reinterpret_cast<Info*>(targetData);

			// Message was sent to an open file handle
			switch(msg.io.type) {
				case IOMsgTypeRead:
				{
					int size = std::min(msg.io.rw.size, info->file.size - info->file.pointer);
					Message_Reply(m, size, reinterpret_cast<char*>(info->file.data) + info->file.pointer, size);
					break;
				}

				case IOMsgTypeSeek:
				{
					info->file.pointer = msg.io.seek.pointer;
					Message_Reply(m, 0, 0, 0);
					break;
				}

				case IOMsgTypeReadDir:
				{
					IOMsgReadDirRet ret;
					int status = 1;
					if((void*)info->dir.header < (void*)__InitFsEnd){
						status = 0;
						strcpy(ret.name, info->dir.header->name);
						info->dir.header = reinterpret_cast<struct InitFsFileHeader*>(reinterpret_cast<char*>(info->dir.header) + sizeof(struct InitFsFileHeader) + info->dir.header->size);
					}
					Message_Reply(m, status, &ret, sizeof(ret));
					break;
				}
			}
		}
	}
}
