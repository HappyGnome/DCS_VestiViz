#pragma once


#ifndef _ASYNCFILTER_H_
#define _ASYNCFILTER_H_

#include<thread>
#include<mutex>
#include <condition_variable>

#include "PostboxInputBase.h"

template <typename Tout>
class AsyncFilter {

	std::thread mWorkerThread;

	void workerFunc() {
		while (process()){}
	}
protected:

	virtual bool process() = 0;

	/**
	 * Cancel processing loop as soon as possible.
	 * Subsequent calls to waitForInput must return false and any blocking calls exit when possible.
	 */
	virtual void cancel() = 0;
public:

	void startProcessing(){
		if (!mWorkerThread.joinable())
		mWorkerThread = std::thread(&AsyncFilter::workerFunc, this);
	}

	void stopProcessing(){
		cancel();
		if (mWorkerThread.joinable()) {
			mWorkerThread.join();
		}
	}

	virtual void setOutput (std::shared_ptr<PostboxInputBase<Tout>> output) = 0;
};
#endif