TARGETS := clientA clientB

clientA_SOURCES := \
	ClientA.c
clientA_BASE_ADDR := 0xa000

clientB_SOURCES := \
	ClientB.c
clientB_BASE_ADDR := 0xb000
