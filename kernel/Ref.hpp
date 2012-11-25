#ifndef REF_H
#define REF_H

/*!
 * \brief Base class for all refcountable objects
 */
class RefObject {
public:
	RefObject() {
		mRefCount = 0;
	}

	void ref() {
		mRefCount++;
		if(mRefCount == 1) {
			onFirstRef();
		}
	}

	void unref() {
		mRefCount--;
		if(mRefCount == 0) {
			onLastRef();
		}
	}

	int refCount() { return mRefCount; }

	virtual void onFirstRef() {}
	virtual void onLastRef() {}

private:
	int mRefCount;
};

/*!
 * \brief Refcounted pointer class
 */
template<typename T>
class Ref {
public:
	Ref() {
		mObject = 0;
	}

	Ref(T *object) {
		mObject = object;
		if(mObject) {
			mObject->ref();
		}
	}

	Ref(const Ref<T> &other) {
		mObject = other.mObject;
		if(mObject) {
			mObject->ref();
		}
	}

	Ref<T> &operator=(const Ref<T> &other) {
		if(mObject) {
			mObject->unref();
		}
		mObject = other.mObject;
		if(mObject) {
			mObject->ref();
		}
	}

	~Ref() {
		mObject->unref();
		mObject = 0;
	}

	T *operator*() { return mObject; }
	T *operator->() { return mObject; }

	T *ptr() { return mObject; }

	operator bool() { return mObject != 0; }

private:
	T *mObject;
};

#endif
