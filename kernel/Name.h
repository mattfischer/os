#ifndef NAME_H
#define NAME_H

#include "Object.h"

int Name_Lookup(const char *name);
void Name_Set(const char *name, int object);

void Name_Init();

#endif