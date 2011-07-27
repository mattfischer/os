#ifndef INIT_FS_FMT_H
#define INIT_FS_FMT_H

#define INIT_FS_NAME_LEN 32
struct InitFsFileHeader {
	int size;
	char name[INIT_FS_NAME_LEN];
};

#endif