#ifndef DEFS_H
#define DEFS_H

#define NULL (void*)0

#define SECTION(sect) __attribute__((section (sect)))

#define SECTION_LOW SECTION(".low")

#endif