#include "Name.h"

#include "List.h"
#include "Slab.h"
#include "Util.h"

struct NameEntry {
	char name[32];
	int object;
	ListEntry2<struct NameEntry> list;
};

SlabAllocator<struct NameEntry> nameSlab;
List2<NameEntry, &NameEntry::list> nameList;

struct NameEntry *findEntry(const char *name)
{
	struct NameEntry *entry;

	for(entry = nameList.head(); entry != NULL; entry = nameList.next(entry)) {
		if(!strcmp(name, entry->name)) {
			return entry;
		}
	}

	return NULL;
}
int Name::lookup(const char *name)
{
	struct NameEntry *entry = findEntry(name);

	if(entry == NULL) {
		return INVALID_OBJECT;
	} else {
		return entry->object;
	}
}

void Name::set(const char *name, int object)
{
	struct NameEntry *entry;

	entry = findEntry(name);

	if(entry == NULL) {
		entry = nameSlab.allocate();

		strcpy(entry->name, name);
		entry->object = object;
		nameList.addTail(entry);
	} else {
		entry->object = object;
	}
}
