#pragma once

#ifndef _DOUBLEINPUTFILTERBASE_H_
#define _DOUBLEINPUTFILTERBASE_H_

#include<mutex>

#include "AsyncFilter.h"
#include "PostboxBase.h"
#include "FilterActionBase.h"

template <typename Tin1,
	typename Tin2,
	typename Tout,
	typename IOWrapper,
	template<typename,typename> typename L1,
	template<typename, typename> typename L2, 
	typename LAlloc1 = std::allocator<Tin1>, 
	typename LAlloc2 = std::allocator<Tin2> >
class DoubleInputFilterBase : public AsyncFilter<IOWrapper> {
private:
	std::shared_ptr<PostboxInputBase<Tout>> mOutput;
	std::mutex mOutputMutex;

	std::shared_ptr<PostboxBase<Tin1, L1,LAlloc1>> mInput1;
	std::shared_ptr<PostboxBase<Tin2, L2, LAlloc2>> mInput2;

	std::unique_ptr <DoubleFilterActionBase<Tin1,Tin2,Tout,L1,L2,LAlloc1,LAlloc2>> mFilterAction;
protected:

	/**
	 * Cancel processing loop as soon as possible.
	 * Subsequent calls to waitForInput must return false and any blocking calls exit when possible.
	 */
	void cancel() final {
		mInput1->cancel();
		mInput2->cancel();
	}

	bool process() final {
		if (!mInput1->waitForPost()) return false;
		if (!mInput2->waitForPost()) return false;
		Tout newLatest = mFilterAction->actOn(mInput1->output(), mInput2->output());

		std::lock_guard<std::mutex> lock(mOutputMutex);
		if (mOutput != nullptr) return mOutput->addDatum(newLatest);
		return true;
	}

public:
	explicit DoubleInputFilterBase(
		const std::shared_ptr<PostboxBase<Tin1, L1, LAlloc1>>& input1, 
		const std::shared_ptr<PostboxBase<Tin2, L1, LAlloc2>>& input2,
		std::unique_ptr <DoubleFilterActionBase<Tin1, Tin2, Tout, L1, L2, LAlloc1, LAlloc2>>&& action)
		:	mInput1(input1), 
			mInput2(input2), 
			mFilterAction(std::move(action)) {};

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
	void unsetOutput() final {
		std::lock_guard<std::mutex> lock(mOutputMutex);
		if (mOutput != nullptr) mOutput->cancel();
		mOutput = nullptr;
	}

	typename IOWrapper::Wrapped getInput(int index) final {
		if (index == 0) return IOWrapper::template Wrap<Tin1>(mInput1);
		else if (index == 1) return IOWrapper::template Wrap<Tin2>(mInput2);
		else return nullptr;
	}

};

#endif