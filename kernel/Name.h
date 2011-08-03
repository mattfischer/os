#ifndef NAME_H
#define NAME_H

#include "Object.h"

struct Object *Name_Lookup(const char *name);
void Name_Set(const char *name, struct Object *object);

void Name_Init();

#endif