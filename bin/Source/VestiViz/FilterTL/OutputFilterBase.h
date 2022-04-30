#pragma once

#ifndef _OUTPUTFILTERBASE_H_
#define _OUTPUTFILTERBASE_H_

#include<mutex>

#include "AsyncFilter.h"
#include "CircPostbox.h"
#include "FilterActionBase.h"

template <typename Tin, typename Tout, typename IOWrapper, template<typename, typename> typename L, typename LAlloc = std::allocator<Tin>>
class OutputFilterBase : public AsyncFilter<IOWrapper> {
private:
	std::shared_ptr<PostboxInputBase<Tout>> mOutput;
	std::shared_ptr<PostboxInputBase<Tout>> mOutputAwaited;//used only in processing thread, set only while holding mOutputMutex
	std::mutex mOutputMutex;

	std::shared_ptr<PostboxBase<Tin, L, LAlloc>> mInput;

	std::unique_ptr < FilterActionBase<Tin, Tout, L, LAlloc>> mFilterAction;
protected:

	/**
	 * Cancel processing loop as soon as possible.
	 * Subsequent calls to waitForInput must return false and any blocking calls exit when possible.
	 */
	void cancel() final {
		mInput->cancel();
		{
			std::lock_guard<std::mutex> lock(mOutputMutex);
			if (mOutputAwaited != nullptr) mOutputAwaited->cancel();
		}
	}

	bool process() final {

		if (!mInput->waitForPost()) return false;

		Tout newLatest = mFilterAction->actOn(mInput->output());

		{
			std::lock_guard<std::mutex> lock(mOutputMutex);
			mOutputAwaited = mOutput;
		}

		if (mOutputAwaited != nullptr){
			return mOutputAwaited->addDatum(newLatest, true);// blocks until datum read
		}
		return true;
	}

public:
	explicit OutputFilterBase(const std::shared_ptr<PostboxBase<Tin, L, LAlloc>>& input, std::unique_ptr <FilterActionBase<Tin, Tout, L, LAlloc>>&& action): mInput(input), mFilterAction(std::move(action)) {};

	bool setOutput(typename IOWrapper::Wrapped&& wrappedInput) final {
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
