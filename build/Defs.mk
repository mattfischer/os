CROSS_COMPILE := arm-none-eabi
CROSS_GCC := $(CROSS_COMPILE)-gcc
CROSS_AS := $(CROSS_COMPILE)-as
CROSS_LD := $(CROSS_COMPILE)-ld
CROSS_AR := $(CROSS_COMPILE)-ar
CROSS_GDB := $(CROSS_COMPILE)-gdb
CROSS_OBJCOPY := $(CROSS_COMPILE)-objcopy
HOST_GCC := gcc
HOST_AS := as
HOST_LD := gcc
HOST_AR := ar

OUTDIR       := out/
CROSS_OBJDIR := $(OUTDIR)obj/
CROSS_BINDIR := $(OUTDIR)bin/
CROSS_DEPDIR := $(OUTDIR)deps/
CROSS_LIBDIR := $(OUTDIR)libs/
HOST_OUTDIR  := $(OUTDIR)host/
HOST_OBJDIR  := $(HOST_OUTDIR)obj/
HOST_BINDIR  := $(HOST_OUTDIR)bin/
HOST_DEPDIR  := $(HOST_OUTDIR)deps/
HOST_LIBDIR  := $(HOST_OUTDIR)libs/

CROSS_CFLAGS := -I . -g
HOST_CFLAGS := -I . -g

CROSS_EXE_EXT :=
CROSS_LIB_EXT := .a

HOST_EXE_EXT := .exe
HOST_LIB_EXT := .lib

