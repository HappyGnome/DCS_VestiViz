#pragma once

#ifndef _OUTPUTFILTERBASE_H_
#define _OUTPUTFILTERBASE_H_

#include<mutex>

#include "AsyncFilter.h"
#include "CircPostbox.h"

template <typename Tin, typename Tout>
class OutputFilterBase : public AsyncFilter<Tout> {
private:
	std::shared_ptr<PostboxInputBase<Tout>> mOutput;
	std::mutex mOutputMutex;

	std::shared_ptr<CircPostbox<Tin>> mInput;
protected:

	/**
	 * Cancel processing loop as soon as possible.
	 * Subsequent calls to waitForInput must return false and any blocking calls exit when possible.
	 */
	void cancel() final {
		mInput->cancel();
	}

	bool process() final {

		if (!mInput->waitForPost()) return false;

		Tout newLatest = processStep(mInput->output());

		std::shared_ptr<PostboxInputBase<Tout>> currentOutput;

		{
			std::lock_guard<std::mutex> lock(mOutputMutex);
			currentOutput = mOutput;
		}

		if (currentOutput != nullptr){
			return currentOutput->addDatum(newLatest, true);// blocks until datum read
		}
		return true;
	}

	/*
	* Process data in the inner buffer and return an updated output.
	* 
	* @param previous Data used to genereate previous datum output. latest Latest datum at the input.
	*/
	virtual Tout processStep(const std::list<Tin>& data) = 0;

public:
	explicit OutputFilterBase(const std::size_t bufSize): mInput(new CircPostbox<Tin>(1,bufSize)) {};

	void setOutput(std::shared_ptr<PostboxInputBase<Tout>> output) final {
		std::lock_guard<std::mutex> lock(mOutputMutex);
		if (mOutput != nullptr) mOutput->cancel();
		mOutput = output;
	}

	std::shared_ptr<PostboxBase<Tin, std::list<Tin>>> getInput() const {
		return mInput;
	}

};

#endif
