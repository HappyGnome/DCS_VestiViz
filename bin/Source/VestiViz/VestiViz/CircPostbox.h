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
	std::condition_variable  mOuterBufFlushCV;

	//Data passes from the outer buffer into the inner buffer
	CircBuf<T> mInnerBuf;

	/**
	 * Call while holding mOuterBufMutex only
     */
	bool doFlushPost() {
		if (mCancelled) return false;
		mInnerBuf.collectfrom(mOuterBuf);
		mReadReceipt = true;
		mOuterBufFlushCV.notify_all();
		return true;
	}

	/**
	* Call while holding mOuterBufMutex only
	*/
	void doAddDatum(const T& input){
		mOuterBuf.push_back(input);
		mOuterBufCV.notify_all();
		mReadReceipt = false;
	}

	/**
	* Call while holding mOuterBufMutex only
	*/
	void doAddDatum(T&& input){
		mOuterBuf.push_back(input);
		mOuterBufCV.notify_all();
		mReadReceipt = false;
	}

	bool doWaitForRead(std::unique_lock<std::mutex>& lock){
		if (mCancelled) return false;
		while (mReadReceipt == false) {
			mOuterBufFlushCV.wait(lock);
			if (mCancelled) return false;
		}
		return true;
	}

public:
	explicit CircPostbox(std::size_t bufSize) :mInnerBuf(bufSize), mOuterBuf(bufSize) {};
	explicit CircPostbox(std::size_t inBufSize, std::size_t outBufSize) :mInnerBuf(inBufSize), mOuterBuf(outBufSize) {};

	bool addDatum(const T& input, bool waitForRead = false) override {
		std::unique_lock<std::mutex> lock(mOuterBufMutex);
		doAddDatum(input);
		if (!waitForRead) return !mCancelled;
		return doWaitForRead(lock);
	}

	bool addDatum(T&& input, bool waitForRead = false) override{
		std::unique_lock<std::mutex> lock(mOuterBufMutex);
		doAddDatum(input);
		if (!waitForRead) return !mCancelled;
		return doWaitForRead(lock);
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