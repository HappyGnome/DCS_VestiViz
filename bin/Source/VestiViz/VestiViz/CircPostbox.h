#pragma once
#ifndef _CIRCPOSTBOX_H_
#define _CIRCPOSTBOX_H_

#include<thread>
#include<mutex>
#include <condition_variable>

#include"CircBuf.h"
#include"PostboxBase.h"

template <typename T>
class CircPostbox : public PostboxBase<T, std::list<T>> {

	bool mCancelled = false;
	bool mReadReceipt = false;
	CircBuf<T> mOuterBuf;
	std::mutex mOuterBufMutex;
	std::condition_variable  mOuterBufCV;

	//Data passes from the outer buffer into the inner buffer
	CircBuf<T> mInnerBuf;

	/**
	 * Call while holding mOuterBufMutex only
     */
	bool doFlushPost() {
		if (mCancelled) return false;
		mInnerBuf.collectfrom(mOuterBuf);
		mReadReceipt = true;
		return true;
	}

	/**
	* Call while holding mOuterBufMutex only
	*/
	void doAddDatum(const T& input){
		mOuterBuf.push_back(input);
		mOuterBufCV.notify_all();
	}

	/**
	* Call while holding mOuterBufMutex only
	*/
	void doAddDatum(T&& input){
		mOuterBuf.push_back(input);
		mOuterBufCV.notify_all();
	}

public:
	explicit CircPostbox(std::size_t bufSize) :mInnerBuf(bufSize), mOuterBuf(bufSize) {};

	void addDatum(const T& input) override {
		std::lock_guard<std::mutex> lock(mOuterBufMutex);
		doAddDatum(input);
	}

	void addDatum(T&& input) override{
		std::lock_guard<std::mutex> lock(mOuterBufMutex);
		doAddDatum(input);
	}

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

	void cancel() override {
		std::lock_guard<std::mutex> lock(mOuterBufMutex);
		mCancelled = true;
		mOuterBufCV.notify_all();
	}

	bool waitForPost() override{
		std::unique_lock<std::mutex> lock(mOuterBufMutex);
		if (mCancelled) return false;
		while (mOuterBuf.empty()) {
			mOuterBufCV.wait(lock);
			if (mCancelled) return false;
		}
		doFlushPost();
		return true;
	}

	const std::list<T>& output() const override{ return mInnerBuf.data(); }

	bool empty() const { return mInnerBuf.empty(); }
};
#endif