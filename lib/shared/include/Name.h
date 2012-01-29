#ifndef SHARED_NAME_H
#define SHARED_NAME_H

#ifdef __cplusplus
extern "C" {
#endif

void Name_Set(const char *name, int obj);
int Name_Lookup(const char *name);
int Name_Open(const char *name);
void Name_Wait(const char *name);

#ifdef __cplusplus
}
#endif

#endif