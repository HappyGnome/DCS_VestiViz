#pragma once


#ifndef _CIRCBUF_H_
#define _CIRCBUF_H_

#include <list>
#include <iterator>
/*#include <vector>
#include<algorithm>*/

//-------------------------------------------------
// List implementation
template <typename T, typename LAlloc = std::allocator<T>> //Buffered object type
class CircBufL {
	std::list<T, LAlloc> mBuf;
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
	explicit CircBufL(std::size_t capacity) : mCapacity(capacity) {}

	void push_back(const T &value) {
		mBuf.push_back(value);
		trim();
	}

	void collectfrom(CircBufL<T, LAlloc> &other) {
		other.trim(mCapacity);

		mBuf.splice(mBuf.end(), other.mBuf);
		
		trim();
	}

	bool empty() const { return mBuf.empty(); }

	std::size_t size() const { return mBuf.size(); }

	typename std::list<T, LAlloc>::const_iterator cbegin() const {
		return mBuf.cbegin();
	}

	typename std::list<T, LAlloc>::const_reverse_iterator crbegin() const {
		return mBuf.crbegin();
	}

	typename std::list<T, LAlloc>::const_iterator cend() const {
		return mBuf.cend();
	}

	typename std::list<T, LAlloc>::const_reverse_iterator crend() const {
		return mBuf.crend();
	}
};

//-------------------------------------------------------------------
//Vector implementation
/*
template <typename T, typename LAlloc> //Buffered object type
class CircBuf<T, std::vector, LAlloc> {
	std::vector<T, LAlloc> mBuf;
	std::size_t mSize = 0;
	std::size_t mWriteTo = 0;

	void trim(std::size_t n) {
		mSize = std::min(mSize, n);
	}
public:
	explicit CircBuf(std::size_t capacity) : mBuf(capacity) {}

	void push_back(const T& value) {
		if (!mBuf.empty()) {
			mBuf[mWriteTo] = value;
			if (mSize < mBuf.size()) mSize++;
			mWriteTo = (mWriteTo + 1) % mBuf.size();
		}
	}

	void push_back(T&& value) {
		if (!mBuf.empty()) {
			mBuf[mWriteTo] = std::move(value);
			if (mSize < mBuf.size()) mSize++;
			mWriteTo = (mWriteTo + 1) % mBuf.size();
		}
	}

	void collectfrom(CircBuf<T, std::vector, LAlloc>& other) {
		std::size_t toCopy = std::min(mBuf.size(), other.mSize);
		std::size_t otherStart1 = (other.mWriteTo + other.mBuf.size() - toCopy) % other.mBuf.size();
		std::size_t otherStop1 = std::min(otherStart1 + toCopy, other.mBuf.size());
		std::size_t otherStepSize1 = otherStop1 - otherStart1;
		std::size_t otherStart2 = 0;
		std::size_t otherStop2 = toCopy - otherStepSize1;

		std::size_t start1 = mWriteTo;
		std::size_t stop1 = std::min(mWriteTo + toCopy, mBuf.size());
		std::size_t stepSize1 = stop1 - start1;
		std::size_t start2 = 0;
		std::size_t stop2 = toCopy - stepSize1;



		if (stepSize1 >= otherStepSize1) {
			std::copy()
		}
	}

	bool empty() const { return mBuf.empty(); }

	typename const_iterator cbegin() const {
		return mBuf.cbegin();
	}

	typename const_iterator crbegin() const {
		return mBuf.crbegin();
	}

	typename const_iterator cend() const {
		return mBuf.cend();
	}

	typename const_iterator crend() const {
		return mBuf.crend();
	}
};

template<typename T, typename LAlloc = std::allocator<T>>
using CircBufV = CircBuf<T, std::vector, LAlloc>;*/
#endif