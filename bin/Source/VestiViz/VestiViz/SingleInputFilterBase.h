#pragma once

#ifndef _SINGLEINPUTFILTERBASE_H_
#define _SINGLEINPUTFILTERBASE_H_

#include<mutex>

#include "AsyncFilter.h"
#include "PostboxBase.h"

template <typename Tin, typename Tout, typename L>
class SingleInputFilterBase : public AsyncFilter<Tout>{
private:
	std::shared_ptr<PostboxInputBase<Tout>> mOutput;
	std::mutex mOutputMutex;

	std::shared_ptr<PostboxBase<Tin,L>> mInput;

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
		Tout newLatest = processStep(mInput -> output() );
		
		std::lock_guard<std::mutex> lock(mOutputMutex);
		if (mOutput != nullptr) return mOutput->addDatum(newLatest);
		return true;
	}

	/*
	* Process data in the inner buffer and return an updated output
	*/
	virtual Tout processStep(const L& data) = 0;

public:
	explicit SingleInputFilterBase(const std::shared_ptr<PostboxBase<Tin, L>>& input):mInput(input){};

	void setOutput(std::shared_ptr<PostboxInputBase<Tout>> output) final {
		std::lock_guard<std::mutex> lock(mOutputMutex);
		mOutput = output;
	}

	std::shared_ptr<PostboxBase<Tin, L>> getInput() const{
		return mInput;
	}

};

#endif