TARGETS := server client

server_SOURCES := \
	Server.c
server_LIBS := system

client_SOURCES := \
	Client.c
client_LIBS := system