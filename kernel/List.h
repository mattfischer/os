#ifndef LIST_H
#define LIST_H

#include <stddef.h>

template<typename T>
struct ListEntryAux {
	T *prev;
	T *next;

	ListEntryAux() {
		prev = NULL;
		next = NULL;
	}
};

template<typename T, ListEntryAux<T> T::*member = NULL>
class ListAux {
public:
	ListAux() {
		mHead = NULL;
		mTail = NULL;
	}

	void init() {
		mHead = NULL;
		mTail = NULL;
	}

	T *head() { return mHead; }
	T *tail() { return mTail; }

	T *next(T *entry) { return (entry->*member).next ? (entry->*member).next : NULL; }
	T *prev(T *entry) { return (entry->*member).prev ? (entry->*member).prev : NULL; }

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

		(entry->*member).next = NULL;
		(entry->*member).prev = NULL;
	}

	bool empty() { return mHead == NULL; }

	bool contains(T *target) {
		for(T *cursor = head(); cursor != NULL; cursor = next(cursor)) {
			if(cursor == target) {
				return true;
			}
		}

		return false;
	}

private:
	T *mHead;
	T *mTail;
};

struct ListEntry {
	ListEntry *prev;
	ListEntry *next;

	ListEntry() {
		prev = NULL;
		next = NULL;
	}
};

template<typename T>
class List {
public:
	List() {
		mHead = NULL;
		mTail = NULL;
	}

	void init() {
		mHead = NULL;
		mTail = NULL;
	}

	T *head() { return mHead ? static_cast<T*>(mHead) : NULL; }
	T *tail() { return mTail ? static_cast<T*>(mTail) : NULL; }

	T *next(ListEntry *entry) { return entry->next ? static_cast<T*>(entry->next) : NULL; }
	T *prev(ListEntry *entry) { return entry->prev ? static_cast<T*>(entry->prev) : NULL; }

	void addHead(ListEntry *entry) {
		entry->prev = NULL;
		entry->next = mHead;

		if(mHead == NULL) {
			mHead = entry;
			mTail = entry;
		} else {
			mHead->prev = entry;
			mHead = entry;
		}
	}

	void addTail(ListEntry *entry) {
		entry->prev = mTail;
		entry->next = NULL;

		if(mHead == NULL) {
			mHead = entry;
			mTail = entry;
		} else {
			mTail->next = entry;
			mTail = entry;
		}
	}

	void addBefore(ListEntry *entry, ListEntry *target) {
		entry->prev = target->prev;
		entry->next = target;

		if(mHead == target) {
			mHead->prev = entry;
			mHead = entry;
		} else {
			target->prev->next = entry;
			target->prev = entry;
		}
	}

	void addAfter(ListEntry *entry, ListEntry *target) {
		entry->prev = target;
		entry->next = target->next;

		if(mTail == target) {
			mTail->next = entry;
			mTail = entry;
		} else {
			target->next->prev = entry;
			target->next = entry;
		}
	}

	void remove(ListEntry *entry) {
		if(entry->prev) {
			entry->prev->next = entry->next;
		}

		if(entry->next) {
			entry->next->prev = entry->prev;
		}

		if(mHead == entry) {
			mHead = entry->next;
		}

		if(mTail == entry) {
			mTail = entry->prev;
		}

		entry->next = NULL;
		entry->prev = NULL;
	}

	T *removeHead() {
		T *ret = head();

		if(ret) {
			remove(ret);
		}

		return ret;
	}

	T *removeTail() {
		T *ret = tail();

		if(ret) {
			remove(ret);
		}

		return ret;
	}

	bool empty() { return mHead == NULL; }

	bool contains(T *target) {
		for(T *cursor = head(); cursor != NULL; cursor = next(cursor)) {
			if(cursor == target) {
				return true;
			}
		}

		return false;
	}

private:
	ListEntry *mHead;
	ListEntry *mTail;
};

#endif