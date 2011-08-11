TARGETS := server client client2

server_SOURCES := \
	Server.c
server_LIBS := system

client_SOURCES := \
	Client.c
client_LIBS := system
client_BASE_ADDR := 0x9000

client2_SOURCES := \
	Client2.c
client2_LIBS := system
client2_BASE_ADDR := 0xa000