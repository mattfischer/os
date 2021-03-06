#ifndef PTE_H
#define PTE_H

#define PAGE_TABLE_SIZE 4096

#define PAGE_TABLE_SECTION_SIZE (1024 * 1024)
#define PAGE_TABLE_SECTION_MASK 0xfff00000
#define PAGE_TABLE_SECTION_SHIFT 20

#define PTE_TYPE_MASK 3
#define PTE_TYPE_DISABLED 0
#define PTE_TYPE_COARSE 1
#define PTE_TYPE_SECTION 2

#define PTE_SECTION_AP_SHIFT 10
#define PTE_SECTION_AP_READ_WRITE (0x3 << PTE_SECTION_AP_SHIFT)
#define PTE_SECTION_AP_READ_ONLY (0x2 << PTE_SECTION_AP_SHIFT)
#define PTE_SECTION_AP_READ_WRITE_PRIV (0x1 << PTE_SECTION_AP_SHIFT)
#define PTE_SECTION_BASE_MASK 0xfff00000
#define PTE_SECTION_BASE_SHIFT 20

#define PTE_COARSE_BASE_MASK 0xfffffc00
#define PTE_COARSE_BASE_SHIFT 10

#define PAGE_L2_TABLE_SIZE 256

#define PTE_L2_TYPE_MASK 3
#define PTE_L2_TYPE_DISABLED 0
#define PTE_L2_TYPE_LARGE 1
#define PTE_L2_TYPE_SMALL 2

#define PTE_L2_AP0_SHIFT 4
#define PTE_L2_AP1_SHIFT 6
#define PTE_L2_AP2_SHIFT 8
#define PTE_L2_AP3_SHIFT 10


#define PTE_L2_AP_READ_WRITE 0x3
#define PTE_L2_AP_READ_ONLY 0x2
#define PTE_L2_AP_READ_WRITE_PRIV 0x1
#define PTE_L2_AP_NONE 0x0
#define PTE_L2_AP_ALL_READ_WRITE 0xff0
#define PTE_L2_AP_ALL_READ_ONLY 0xCC0
#define PTE_L2_AP_ALL_READ_WRITE_PRIV 0x550
#define PTE_L2_AP_ALL_NONE 0x0

#define PTE_L2_BASE_MASK 0xfffff000
#define PTE_L2_BASE_SHIFT 12

#endif
