#include <Object.h>
#include <Message.h>
#include <System.h>
#include <Name.h>

#include <fcntl.h>
#include <unistd.h>

#include <kernel/include/IOFmt.h>
#include <kernel/include/NameFmt.h>

#include <algorithm>

struct Info {
	int obj;
};

int main(int argc, char *argv[])
{
	int server = Object_Create(OBJECT_INVALID, NULL);
	int uart = open(argv[2], O_RDWR);

	Name_Set(argv[1], server);

	while(1) {
		union {
			union IOMsg io;
			union NameMsg name;
		} msg;
		struct MessageInfo info;
		int m;

		m = Object_Receive(server, &msg, sizeof(msg));

		if(m == 0) {
			switch(msg.name.event.type) {
				case SysEventObjectClosed:
					{
						struct Info *info = (struct Info*)msg.name.event.targetData;
						Object_Release(info->obj);
						delete info;
						break;
					}
			}
			continue;
		}

		Message_Info(m, &info);

		if(info.targetData == NULL) {
			switch(msg.name.msg.type) {
				case NameMsgTypeOpen:
				{
					struct Info *info = new Info;
					int obj = Object_Create(server, info);
					info->obj = obj;

					struct BufferSegment segs[] = { &obj, sizeof(obj) };
					struct MessageHeader hdr = { segs, 1, 0, 1 };

					Message_Replyx(m, 0, &hdr);
					break;
				}
			}
		} else {
			switch(msg.io.msg.type) {
				case IOMsgTypeWrite:
				{
					char buffer[256];
					int sent;
					int headerSize;

					headerSize = offsetof(union IOMsg, msg.u.rw) + sizeof(msg.io.msg.u.rw);
					sent = 0;
					while(sent < msg.io.msg.u.rw.size) {
						int size;

						size = Message_Read(m, buffer, headerSize + sent, sizeof(buffer));
						write(uart, buffer, size);
						sent += size;
					}
					Message_Reply(m, sent, NULL, 0);
					break;
				}

				case IOMsgTypeRead:
				{
					char buffer[256];
					int n = 0;
					char c;
					while(true) {
						read(uart, &c, 1);
						if(c == '\r') {
							buffer[n++] = '\n';
							break;
						} else if(c == 127) {
							if(n > 0) {
								n--;
								write(uart, "\x8 \x8", 3);
							}
						} else {
							buffer[n++] = c;
							write(uart, &c, 1);
						}
					}
					Message_Reply(m, n, buffer, n);
				}
			}
		}
	}
}