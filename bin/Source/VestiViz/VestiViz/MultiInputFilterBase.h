#pragma once

#ifndef _MULTIINPUTFILTERBASE_H_
#define _MULTIINPUTFILTERBASE_H_

#include<mutex>

#include "AsyncFilter.h"
#include "PostboxBase.h"
#include "FilterActionBase.h"

template <typename Tin, typename Tout, typename... L>
class SingleInputFilterBase : public AsyncFilter<Tout> {
private:
	std::shared_ptr<PostboxInputBase<Tout>> mOutput;
	std::mutex mOutputMutex;

	std::array<std::shared_ptr<PostboxBase<Tin, L>>,N> mInput;

	std::unique_ptr <MultiFilterActionBase<Tout, L>> mFilterAction;
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
		Tout newLatest = mFilterAction->actOn(mInput->output());

		std::lock_guard<std::mutex> lock(mOutputMutex);
		if (mOutput != nullptr) return mOutput->addDatum(newLatest);
		return true;
	}

public:
	explicit SingleInputFilterBase(const std::shared_ptr<PostboxBase<Tin, L>>& input, std::unique_ptr <FilterActionBase<Tout, L>>&& action) :mInput(input), mFilterAction(std::move(action)) {};

	void setOutput(std::shared_ptr<PostboxInputBase<Tout>> output) final {
		std::lock_guard<std::mutex> lock(mOutputMutex);
		mOutput = output;
	}

	std::shared_ptr<PostboxBase<Tin, L>> getInput() const {
		return mInput;
	}

};

#endif