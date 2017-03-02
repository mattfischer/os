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

int fileServer;
int fileChannel;

// Path at which to register the InitFS
#define PREFIX "/boot"

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
		struct MessageInfo messageInfo;
		int m = Channel_Receive(fileChannel, &msg, sizeof(msg), &messageInfo);

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
			}
			continue;
		}

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

	Task *task = Kernel::process()->newTask();
	task->start(server, 0);
}

/*!
 * \brief Retrieve the name server object
 * \return Name server
 */
int InitFs::nameServer()
{
	return fileServer;
}
