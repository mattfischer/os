#include "Name.h"

#include "List.h"
#include "Slab.h"
#include "Util.h"

struct NameEntry {
	char name[32];
	struct Object *object;
	struct ListEntry list;
};

struct SlabAllocator nameSlab;
LIST(struct NameEntry) nameList;

struct NameEntry *findEntry(const char *name)
{
	struct NameEntry *entry;

	LIST_FOREACH(nameList, entry, struct NameEntry, list) {
		if(!strcmp(name, entry->name)) {
			return entry;
		}
	}

	return NULL;
}
struct Object *Name_Lookup(const char *name)
{
	struct NameEntry *entry = findEntry(name);

	if(entry == NULL) {
		return NULL;
	} else {
		return entry->object;
	}
}

void Name_Set(const char *name, struct Object *object)
{
	struct NameEntry *entry;

	entry = findEntry(name);

	if(entry == NULL) {
		entry = (struct NameEntry*)Slab_Allocate(&nameSlab);

		strcpy(entry->name, name);
		entry->object = object;
		LIST_ADD_TAIL(nameList, entry->list);
	} else {
		entry->object = object;
	}
}

void Name_Init()
{
	Slab_Init(&nameSlab, sizeof(struct NameEntry));
	LIST_INIT(nameList);
}