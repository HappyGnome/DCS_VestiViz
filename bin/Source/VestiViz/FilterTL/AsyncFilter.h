#pragma once


#ifndef _ASYNCFILTER_H_
#define _ASYNCFILTER_H_

#include<thread>
#include<mutex>
#include <condition_variable>

#include "PostboxInputBase.h"
#include "ErrorStack.h"

template<typename IOWrapper>
class AsyncFilter {

	std::shared_ptr<ErrorStack> mLog;
protected:

	void log(const std::exception& e) {
		if (mLog != nullptr) {
			mLog->push_exception(e);
		}
	}
	void log(const std::string& msg) {
		if (mLog != nullptr) {
			mLog->push_message(msg);
		}
	}
private:

	std::thread mWorkerThread;
	void workerFunc() {
		try {
			while (process()) {}
		}
		catch (const std::exception& e) {
			log(e);
		}
	}
protected:

	virtual bool process() = 0;

	/**
	 * Cancel processing loop as soon as possible.
	 * Subsequent calls to waitForInput must return false and any blocking calls exit when possible.
	 */
	virtual void cancel() = 0;
public:

	explicit AsyncFilter(std::shared_ptr<ErrorStack> log = nullptr) :mLog(log) {};

	virtual ~AsyncFilter() = default;

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

	virtual bool setOutput (typename IOWrapper::Wrapped&& wrappedInput) = 0;
	virtual void unsetOutput() = 0;
	virtual typename IOWrapper::Wrapped getInput(int index) = 0;
};
#endif