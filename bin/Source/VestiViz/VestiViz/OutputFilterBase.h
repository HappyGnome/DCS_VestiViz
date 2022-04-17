#pragma once


#pragma once

#ifndef _OUTPUTFILTERBASE_H_
#define _OUTPUTFILTERBASE_H_

#include<mutex>

#include "AsyncDataHandler.h"

template <typename Tin, typename Tout>
class OutputFilterBase : public AsyncDataHandler<Tin> {
private:

	Tout mLatest;
	Tout mLatestInput;
	std::mutex mLatestMutex;
	bool mHasLatest = false;
protected:

	virtual void processInput() final {

		Tout newLatest = processStep();

		{//mNextProcessorMutex lock scope
			std::lock_guard<std::mutex> lock(mNextProcessorMutex);
			if (mNextProcessor.get() != nullptr) {
				mNextProcessor.get()->addDatum(newLatest);
			}
		}

		{//mLatestMutex lock scope
			std::lock_guard<std::mutex> lock(mLatestMutex);
			mLatest = std::move(newLatest);
			mHasLatest = true;
		}
	}

	/*
	* Process data in the inner buffer and return an updated output
	*/
	virtual Tout processStep() = 0;

public:
	explicit OutputFilterBase(std::size_t bufSize, Tout&& defaultLatest) :AsyncDataHandler<Tin>(1), mLatest(defaultLatest) {};

	OutputFilterBase() = delete;
	OutputFilterBase(const OutputFilterBase& other) = delete;
	OutputFilterBase(OutputFilterBase&& other) = delete;
	OutputFilterBase& operator =(OutputFilterBase&& other) = delete;
	OutputFilterBase& operator =(const OutputFilterBase& other) = delete;
	virtual ~OutputFilterBase() = default;

	bool getLatest(Tout& output) const {
		std::lock_guard<std::mutex> lock(mLatestMutex);
		if (mHasLatest) {
			output = mLatest;
			return true;
		}
		else return false;
	}

	void setNextProcessor(std::shared_ptr<AsyncDataHandler<Tout>> next) {
		{//mNextProcessorMutex lock scope
			std::lock_guard<std::mutex> lock(mNextProcessorMutex);
			mNextProcessor = next;
		}
	}

	void unsetNextProcessor() {
		{//mNextProcessorMutex lock scope
			std::lock_guard<std::mutex> lock(mNextProcessorMutex);
			mNextProcessor = std::shared_ptr<AsyncDataHandler<Tout>>();
		}
	}
};

#endif
