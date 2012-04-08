#ifndef SHARED_IO_H
#define SHARED_IO_H

#ifdef __cplusplus
extern "C" {
#endif

int File_Write(int obj, void *buffer, int size);
int File_Read(int obj, void *buffer, int size);
void File_Seek(int obj, int pointer);

int File_ReadDir(int obj, char *name);

#ifdef __cplusplus
}
#endif

#endif