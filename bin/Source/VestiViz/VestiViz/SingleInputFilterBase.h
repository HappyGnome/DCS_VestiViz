#pragma once

#ifndef _SINGLEINPUTFILTERBASE_H_
#define _SINGLEINPUTFILTERBASE_H_

#include<mutex>

#include "AsyncFilter.h"
#include "PostboxBase.h"
#include "FilterActionBase.h"
#include "PostboxWrapper.h"

template <typename Tin, typename Tout,typename IOWrapper,template<typename, typename> typename L, typename LAlloc = std::allocator<Tin>>
class SingleInputFilterBase : public AsyncFilter<IOWrapper>{
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

	bool setOutput(typename IOWrapper::Wrapped&& wrappedInput) final{
		auto unwrapped = IOWrapper::template Unwrap<Tout>(std::move(wrappedInput));
		if (unwrapped != nullptr) {
			std::lock_guard<std::mutex> lock(mOutputMutex);
			if (mOutput != nullptr) mOutput->cancel();
			mOutput = unwrapped;
			return true;
		}
		return false;
	}
	
	typename IOWrapper::Wrapped getInput(int index) final {
		if (index != 0)return nullptr;
		return IOWrapper::template Wrap<Tin>(mInput);
	}
};

#endif