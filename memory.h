#ifndef MEMORY_H
#define MEMORY_H

#define PTE_TYPE_DISABLED 0
#define PTE_TYPE_COARSE 1
#define PTE_TYPE_SECTION 2

#define PTE_AP_SHIFT 10
#define PTE_AP_READ_WRITE (0x3 << PTE_AP_SHIFT)

#define PTE_BASE_SHIFT 20

#define PAGE_TABLE_SIZE 4096

extern unsigned initMap[PAGE_TABLE_SIZE];

#endif