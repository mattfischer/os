TARGETS := system

system_SOURCES := \
	Map.c \
	Message.c \
	Name.c \
	Object.c \
	SwiAsm.s \
	Util.c
system_TYPE := LIBRARY