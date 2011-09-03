TARGETS := system

system_SOURCES := \
	KernelObject.c \
	Map.c \
	Message.c \
	NewlibFuncs.c \
	Object.c \
	Spawn.c \
	Start.c \
	SwiAsm.s \
	Yield.c

system_TYPE := LIBRARY