#ifndef LIST_H
#define LIST_H

#include <stddef.h>

struct ListEntry {
	struct ListEntry *prev;
	struct ListEntry *next;
};

struct List {
	struct ListEntry *head;
	struct ListEntry *tail;
};

#define CONTAINER_OF(ptr, type, member) (type*)((char*)ptr - offsetof(type, member))

#define LIST(type) struct List

#define LIST_INIT(list) \
	(list).head = NULL; \
	(list).tail = NULL;

#define LIST_HEAD(list, type, member) \
	(((list).head == NULL) ? NULL : CONTAINER_OF((list).head, type, member))

#define LIST_TAIL(list, type, member) \
	(((list).tail == NULL) ? NULL : CONTAINER_OF((list).tail, type, member))

#define LIST_ADD_TAIL(list, entry) \
	(entry).prev = (list).head; \
	(entry).next = NULL; \
	if((list).head == NULL) { \
		(list).head = &(entry); \
		(list).tail = &(entry); \
	} else { \
		(list).tail->next = &(entry); \
		(list).tail = &(entry); \
	}

#define LIST_ADD_HEAD(list, entry) \
	(entry).prev = NULL; \
	(entry).next = (list).head; \
	if((list).head == NULL) { \
		(list).head = &(entry); \
		(list).tail = &(entry); \
	} else { \
		(list).head->prev = &(entry); \
		(list).head = &(entry); \
	}

#define LIST_ADD_BEFORE(list, entry, target) \
	(entry).prev = (target).prev; \
	(entry).next = &(target); \
	if((list).head == &(target)) { \
		(list).head->prev = &(entry); \
		(list).head = &(entry); \
	} else { \
		(target).prev->next = &(entry); \
		(target).prev = &(entry); \
	}

#define LIST_ADD_AFTER(list, entry, target) \
	(entry).prev = &(target); \
	(entry).next = (target).next; \
	if((list).tail == &(target)) { \
		(list).tail->next = &(entry); \
		(list).tail = &(entry); \
	} else { \
		(target).next->prev = &(entry); \
		(target).next = &(entry); \
	}

#define LIST_REMOVE(list, entry) \
	if((entry).prev) { \
		(entry).prev->next = (entry).next; \
	} \
	if((entry).next) { \
		(entry).next->prev = (entry).prev; \
	} \
	if((list).head == &(entry)) { \
		(list).head = (entry).next; \
	} \
	if((list).tail == &(entry)) { \
		(list).tail = (entry).prev; \
	} \
	LIST_ENTRY_CLEAR(entry);

#define LIST_ENTRY(ptr, type, member) (type*)CONTAINER_OF(ptr, type, member)

#define LIST_EMPTY(list) ((list).head == NULL)

#define LIST_ENTRY_CLEAR(entry) \
	(entry).next = NULL; \
	(entry).prev = NULL;

#define LIST_FOREACH(list, cursor, type, member) \
	for((cursor) = LIST_ENTRY((list).head, type, member); &((cursor)->member) != NULL; cursor = LIST_ENTRY((cursor)->member.next, type, member))

#define LIST_FOREACH_CAN_REMOVE(list, cursor, extra, type, member) \
	for((cursor) = LIST_ENTRY((list).head, type, member), (extra) = LIST_ENTRY((cursor)->member.next, type, member); &((cursor)->member) != NULL; cursor = extra)

#endif