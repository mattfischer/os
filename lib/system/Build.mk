TARGETS := system

system_SOURCES := \
	Map.c \
	Message.c \
	Name.c \
	NewlibFuncs.c \
	Object.c \
	Start.c \
	SwiAsm.s

system_TYPE := LIBRARY