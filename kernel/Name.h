#ifndef NAME_H
#define NAME_H

class Name {
public:
	static int lookup(const char *name);
	static void set(const char *name, int object);
};

#endif