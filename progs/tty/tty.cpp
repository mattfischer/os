#include <Object.h>
#include <Message.h>
#include <System.h>
#include <Name.h>
#include <Channel.h>

#include <fcntl.h>
#include <unistd.h>

#include <kernel/include/IOFmt.h>
#include <kernel/include/NameFmt.h>

#include <algorithm>

enum {
	TypeRoot,
	TypeConnection
};

int main(int argc, char *argv[])
{
	int channel = Channel_Create();
	int server = Object_Create(channel, TypeRoot);
	int uart = open(argv[2], O_RDWR);

	Name_Set(argv[1], server);

	while(1) {
		union {
			struct IOMsg io;
			struct NameMsg name;
		} msg;
		unsigned targetData;
		int m;

		m = Channel_Receive(channel, &msg, sizeof(msg), &targetData);

		if(m == 0) {
			continue;
		}

		switch(targetData) {
		case TypeRoot:
			switch(msg.name.type) {
				case NameMsgTypeOpen:
				{
					int obj = Object_Create(channel, TypeConnection);
					Message_Replyh(m, 0, &obj, sizeof(obj), 0, 1);
					Object_Release(obj);
					break;
				}
			}
			break;

		case TypeConnection:
			switch(msg.io.type) {
				case IOMsgTypeWrite:
				{
					char buffer[256];
					int sent;
					int headerSize;

					headerSize = offsetof(struct IOMsg, rw) + sizeof(msg.io.rw);
					sent = 0;
					while(sent < msg.io.rw.size) {
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