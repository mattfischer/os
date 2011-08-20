#ifndef LIST_H
#define LIST_H

#include <stddef.h>

template<typename T>
struct ListEntryAux {
	T *prev;
	T *next;
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

struct ListEntry {
	ListEntry *prev;
	ListEntry *next;
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

	T *head() { return static_cast<T*>(mHead); }
	T *tail() { return static_cast<T*>(mTail); }

	T *next(ListEntry *entry) { return static_cast<T*>(entry->next); }
	T *prev(ListEntry *entry) { return static_cast<T*>(entry->prev); }

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
	}

	bool empty() { return mHead == NULL; }

private:
	ListEntry *mHead;
	ListEntry *mTail;
};

#endif