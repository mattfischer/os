#ifndef NAME_H
#define NAME_H

#include "Object.h"

class Name {
public:
	static int lookup(const char *name);
	static void set(const char *name, int object);
};

#endif