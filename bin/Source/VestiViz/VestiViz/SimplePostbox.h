#pragma once
#ifndef _SIMPLEPOSTBOX_H_
#define _SIMPLEPOSTBOX_H_

#include<thread>
#include<mutex>
#include <condition_variable>

#include"PostboxBase.h"

template <typename T>
class SimplePostbox : public PostboxBase<T,T> {

	bool mCancelled = false;
	bool mReadReceipt = false;
	bool mHasNewData = false;
	T mOuterBuf;
	std::mutex mOuterBufMutex;
	std::condition_variable  mOuterBufCV;	

	//Data passes from the outer buffer into the inner buffer
	T mInnerBuf;

	bool mHasData = false;

	/**
	* Call while holding mOuterBufMutex only
	*/
	bool doFlushPost(){
		if (mCancelled) return false;
		std::swap(mInnerBuf, mOuterBuf);
		mReadReceipt = true;
		mHasNewData = false;
		return true;
	}

	/**
	* Call while holding mOuterBufMutex only
	*/
	void doAddDatum(const T& input) {
		mOuterBuf = input;
		mHasData = true;
		mHasNewData = true;
		mOuterBufCV.notify_all();
	}

	/**
	* Call while holding mOuterBufMutex only
	*/
	void doAddDatum(T&& input) {
		mOuterBuf = input;
		mHasData = true;
		mHasNewData = true;
		mOuterBufCV.notify_all();
	}

public:
	bool addDatumIfReadMatches(T&& input, bool match) override {
		std::lock_guard<std::mutex> lock(mOuterBufMutex);
		if (match != mReadReceipt) return false;
		doAddDatum(input);
		return true;
	}

	bool addDatumIfReadMatches(T& input, bool match) override {
		std::lock_guard<std::mutex> lock(mOuterBufMutex);
		if (match != mReadReceipt) return false;
		doAddDatum(input);
		return true;
	}

	void resetRead() override {
		std::lock_guard<std::mutex> lock(mOuterBufMutex);
		mReadReceipt = false;
	}

	bool flushPost() override {
		std::lock_guard<std::mutex> lock(mOuterBufMutex);
		return doFlushPost();
	}

	void addDatum(const T& input) override {
		std::lock_guard<std::mutex> lock(mOuterBufMutex);
		doAddDatum(input);
	}

	void addDatum(T&& input) override {
		std::lock_guard<std::mutex> lock(mOuterBufMutex);
		doAddDatum(input);
	}

	void cancel() override {
		std::lock_guard<std::mutex> lock(mOuterBufMutex);
		mCancelled = true;
		mOuterBufCV.notify_all();
	}

	bool waitForPost() override {
		std::unique_lock<std::mutex> lock(mOuterBufMutex);
		if (mCancelled) return false;
		while (!mHasNewData) {
			mOuterBufCV.wait(lock);
			if (mCancelled) return false;
		}
		return doFlushPost();
	}

	const T& output() const override { return mInnerBuf; }

	bool empty() const { return !mHasData; }
};
#endif