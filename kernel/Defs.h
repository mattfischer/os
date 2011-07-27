#ifndef DEFS_H
#define DEFS_H

#include <stddef.h>

#define SECTION(sect) __attribute__((section (sect)))
#define SECTION_LOW SECTION(".low")

#endif