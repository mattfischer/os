TARGETS := server client

server_SOURCES := \
	Server.c \
	Shared.c \
	SharedAsm.s

client_SOURCES := \
	Client.c \
	Shared.c \
	SharedAsm.s