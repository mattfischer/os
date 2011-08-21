#include "Name.h"

#include "List.h"
#include "Slab.h"
#include "Util.h"
#include "Object.h"

struct NameEntry : public ListEntry {
	char name[32];
	int object;
};

SlabAllocator<struct NameEntry> nameSlab;
List<NameEntry> nameList;

struct NameEntry *findEntry(const char *name)
{
	for(struct NameEntry *entry = nameList.head(); entry != NULL; entry = nameList.next(entry)) {
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
	struct NameEntry *entry = findEntry(name);

	if(entry == NULL) {
		entry = nameSlab.allocate();

		strcpy(entry->name, name);
		entry->object = object;
		nameList.addTail(entry);
	} else {
		entry->object = object;
	}
}
