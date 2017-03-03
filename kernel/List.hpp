#ifndef LIST_H
#define LIST_H

/*!
 * \brief Intrusive list entry type.
 *
 * Use this type when an item only needs to be contained in
 * one list.  Inherit the item type from this class, and use
 * the class List.
 */
struct ListEntry {
	ListEntry *prev; //!< Previous item
	ListEntry *next; //!< Next item

	ListEntry() {
		prev = 0;
		next = 0;
	}
};

/*!
 * \brief Intrusive list.  Uses ListEntry to store list pointers
 */
template<typename T>
class List {
public:
	//! Constructor
	List() {
		mHead = 0;
		mTail = 0;
	}

	//! Initialize
	void init() {
		mHead = 0;
		mTail = 0;
	}

	/*!
	 * \brief List head
	 * \return Head
	 */
	T *head() { return mHead ? static_cast<T*>(mHead) : 0; }
	/*!
	 * \brief List tail
	 * \return Tail
	 */
	T *tail() { return mTail ? static_cast<T*>(mTail) : 0; }

	/*!
	 * \brief Get next item in list
	 * \param entry Current item
	 * \return Next item
	 */
	T *next(ListEntry *entry) { return entry->next ? static_cast<T*>(entry->next) : 0; }
	/*!
	 * \brief Get previous item in list
	 * \param entry Current item
	 * \return Previous item
	 */
	T *prev(ListEntry *entry) { return entry->prev ? static_cast<T*>(entry->prev) : 0; }

	/*!
	 * \brief Add item to head of list
	 * \param entry Item to add
	 */
	void addHead(ListEntry *entry) {
		entry->prev = 0;
		entry->next = mHead;

		if(mHead == 0) {
			mHead = entry;
			mTail = entry;
		} else {
			mHead->prev = entry;
			mHead = entry;
		}
	}

	/*!
	 * \brief Add item to tail of list
	 * \param entry Item to add
	 */
	void addTail(ListEntry *entry) {
		entry->prev = mTail;
		entry->next = 0;

		if(mHead == 0) {
			mHead = entry;
			mTail = entry;
		} else {
			mTail->next = entry;
			mTail = entry;
		}
	}

	/*!
	 * \brief Add item before another item
	 * \param entry Item to add
	 * \param target Item to add before
	 */
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

	/*!
	 * \brief Add item after another item
	 * \param entry Item to add
	 * \param target Item to add after
	 */
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

	/*!
	 * \brief Remove item
	 * \param entry Item to remove
	 */
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

		entry->next = 0;
		entry->prev = 0;
	}

	/*!
	 * \brief Remove head of list
	 * \return Removed item
	 */
	T *removeHead() {
		T *ret = head();

		if(ret) {
			remove(ret);
		}

		return ret;
	}

	/*!
	 * \brief Remove tail of list
	 * \return Removed item
	 */
	T *removeTail() {
		T *ret = tail();

		if(ret) {
			remove(ret);
		}

		return ret;
	}

	/*!
	 * \brief Determines if list is empty
	 * \return True if empty, false otherwise
	 */
	bool empty() { return mHead == 0; }

	/*!
	 * \brief Determine if an item is contained in the list
	 * \param target Item
	 */
	bool contains(T *target) {
		for(T *cursor = head(); cursor != 0; cursor = next(cursor)) {
			if(cursor == target) {
				return true;
			}
		}

		return false;
	}

private:
	ListEntry *mHead; //!< Head of list
	ListEntry *mTail; //!< Tail of list
};

#endif
