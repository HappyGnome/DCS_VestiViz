#pragma once

#ifndef _SINGLEINPUTFILTERBASE_H_
#define _SINGLEINPUTFILTERBASE_H_

#include<mutex>

#include "AsyncFilter.h"
#include "PostboxBase.h"
#include "FilterActionBase.h"

template <typename Tin, typename Tout, template<typename, typename> typename L, typename LAlloc = std::allocator<Tin>>
class SingleInputFilterBase : public AsyncFilter<Tout>{
private:
	std::shared_ptr<PostboxInputBase<Tout>> mOutput;
	std::mutex mOutputMutex;

	std::shared_ptr<PostboxBase<Tin,L,LAlloc>> mInput;

	std::unique_ptr <FilterActionBase<Tin,Tout, L,LAlloc>> mFilterAction;
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
		Tout newLatest = mFilterAction->actOn(mInput -> output() );
		
		std::lock_guard<std::mutex> lock(mOutputMutex);
		if (mOutput != nullptr) return mOutput->addDatum(newLatest);
		return true;
	}

public:
	explicit SingleInputFilterBase(const std::shared_ptr<PostboxBase<Tin, L, LAlloc>>& input, std::unique_ptr <FilterActionBase<Tin,Tout, L, LAlloc>>&& action):mInput(input), mFilterAction(std::move(action)) {};

	void setOutput(std::shared_ptr<PostboxInputBase<Tout>> output) final {
		std::lock_guard<std::mutex> lock(mOutputMutex);
		mOutput = output;
	}

	std::shared_ptr<PostboxBase<Tin, L, LAlloc>> getInput() const{
		return mInput;
	}

};

#endif