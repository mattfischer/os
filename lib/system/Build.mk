TARGETS := system

system_SOURCES := \
	IO.c \
	Map.c \
	Message.c \
	Name.c \
	NewlibFuncs.c \
	Object.c \
	ProcessManager.c \
	Spawn.c \
	Start.c \
	SwiAsm.s \
	Yield.c

system_TYPE := LIBRARY