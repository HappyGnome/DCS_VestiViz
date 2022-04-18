#pragma once


#ifndef _CIRCBUF_H_
#define _CIRCBUF_H_

#include <list>
#include <iterator>

template <typename T> //Buffered object type
class CircBuf {
	std::list<T> mBuf;
	std::size_t mCapacity;

	void trim(std::size_t n){
		if (mBuf.size() > n) {
			auto last = mBuf.cbegin();
			std::advance(last, mBuf.size() - n);
			mBuf.erase(mBuf.cbegin(), last);
		}
	}

	void trim() {
		trim(mCapacity);
	}
public:
	explicit CircBuf(std::size_t capacity) : mCapacity(capacity) {}

	void push_back(const T &value) {
		mBuf.push_back(value);
		trim();
	}

	void collectfrom(CircBuf<T> &other) {
		other.trim(mCapacity);

		mBuf.splice(mBuf.end(), other.mBuf);
		
		trim();
	}

	typename const std::list<T>& data() const{
		return mBuf;
	}

	bool empty() const { return mBuf.empty(); }
};

#endif