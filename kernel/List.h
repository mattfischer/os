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
	(entry).prev = (list).tail; \
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

template<typename T>
struct ListEntry2 {
	T *prev;
	T *next;
};

template<typename T, ListEntry2<T> T::*member>
class List2 {
public:
	List2() {
		mHead = NULL;
		mTail = NULL;
	}

	void init() {
		mHead = NULL;
		mTail = NULL;
	}

	T *head() { return mHead; }
	T *tail() { return mTail; }

	T *next(T *entry) { return (entry->*member).next; }
	T *prev(T *entry) { return (entry->*member).prev; }

	void addHead(T *entry) {
		(entry->*member).prev = NULL;
		(entry->*member).next = mHead;
		if(mHead == NULL) {
			mHead = entry;
			mTail = entry;
		} else {
			(mHead->*member).prev = entry;
			mHead = entry;
		}
	}

	void addTail(T *entry) {
		(entry->*member).prev = mTail;
		(entry->*member).next = NULL;
		if(mHead == NULL) {
			mHead = entry;
			mTail = entry;
		} else {
			(mTail->*member).next = entry;
			mTail = entry;
		}
	}

	void addBefore(T *entry, T *target) {
		(entry->*member).prev = (target->*member).prev;
		(entry->*member).next = target;
		if(mHead == target) {
			(mHead->*member).prev = entry;
			mHead = entry;
		} else {
			((target->*member).prev->*member).next = entry;
			(target->*member).prev = entry;
		}
	}

	void addAfter(T *entry, T *target) {
		(entry->*member).prev = target;
		(entry->*member).next = (target->*member).next;
		if(mTail == target) {
			(mTail->*member).next = entry;
			mTail = entry;
		} else {
			((target->*member).next->*member).prev = entry;
			(target->*member).next = entry;
		}
	}

	void remove(T *entry) {
		if((entry->*member).prev) {
			((entry->*member).prev->*member).next = (entry->*member).next;
		}
		if((entry->*member).next) {
			((entry->*member).next->*member).prev = (entry->*member).prev;
		}
		if(mHead == entry) {
			mHead = (entry->*member).next;
		}
		if(mTail == entry) {
			mTail = (entry->*member).prev;
		}
	}

	bool empty() { return mHead == NULL; }

private:
	T *mHead;
	T *mTail;
};

#endif