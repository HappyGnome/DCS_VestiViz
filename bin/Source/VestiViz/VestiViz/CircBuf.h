#pragma once


#ifndef _CIRCBUF_H_
#define _CIRCBUF_H_

#include <list>
#include <iterator>

template <typename T> //Buffered object type
class CircBuf {
	std::list<T> mBuf;
	std::size_t mCapacity;

	void trim(int n = mCapacity){
		int excess = mBuf.size() - mCapacity;
		if (excess > 0) {
			mBuf.erase(mBuf.cbegin,std::advance(mBuf.cbegin,excess));
		}
	}
public:

	explicit CircBuf(std::size_t capacity) : mCapacity(capacity) {}

	void push_back(T &value const) {
		mBuf.push_back(value);
		trim();
	}

	void collectfrom(CircBuf<T> &other) {
		other.trim(mCapacity);

		mBuf.splice(mBuf.end(), other.mBuf);
		
		trim();
	}

	std::iterator<T> cbegin() const{
		return mBuf.cbegin();
	}

	std::iterator<T> cend() const {
		return mBuf.cend();
	}

	bool empty() const { return mBuf.empty(); }
};

#endif