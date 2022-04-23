#pragma once

#ifndef _DOUBLEINPUTFILTERBASE_H_
#define _DOUBLEINPUTFILTERBASE_H_

#include<mutex>

#include "AsyncFilter.h"
#include "PostboxBase.h"
#include "FilterActionBase.h"

template <typename Tin1,
	typename Tin2 ,
	typename Tout,
	template<typename,typename> typename L1,
	template<typename, typename> typename L2, 
	typename LAlloc1 = std::allocator<Tin1>, 
	typename LAlloc2 = std::allocator<Tin2> >
class DoubleInputFilterBase : public AsyncFilter<Tout> {
private:
	std::shared_ptr<PostboxInputBase<Tout>> mOutput;
	std::mutex mOutputMutex;

	std::shared_ptr<PostboxBase<Tin1, L1<Tin1,LAlloc1>>> mInput1;
	std::shared_ptr<PostboxBase<Tin2, L2<Tin2, LAlloc2>>> mInput2;

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
	explicit SingleInputFilterBase(
		const std::shared_ptr<PostboxBase<Tin1, L1<Tin1, LAlloc1>>>& input1, 
		const std::shared_ptr<PostboxBase<Tin2, L1<Tin2, LAlloc2>>>& input2,
		std::unique_ptr <FilterActionBase<Tout, L>>&& action) 
		:	mInput1(input1), 
			mInput2(input2), 
			mFilterAction(std::move(action)) {};

	void setOutput(std::shared_ptr<PostboxInputBase<Tout>> output) final {
		std::lock_guard<std::mutex> lock(mOutputMutex);
		mOutput = output;
	}

	std::shared_ptr<PostboxBase<Tin1, L1<Tin1, LAlloc1>>> getInput1() const {
		return mInput1;
	}
	std::shared_ptr<PostboxBase<Tin2, L1<Tin2, LAlloc2>>> getInput2() const {
		return mInput2;
	}

};

#endif