TARGETS := system

system_SOURCES := \
	IO.c \
	KernelObject.c \
	Map.c \
	Message.c \
	Name.c \
	NewlibFuncs.c \
	Object.c \
	Spawn.c \
	Start.c \
	SwiAsm.s \
	Yield.c

system_TYPE := LIBRARY