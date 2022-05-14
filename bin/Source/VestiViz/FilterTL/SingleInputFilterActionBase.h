#pragma once

#ifndef _SINGLEINPUTFILTERACTIONBASE_H_
#define _SINGLEINPUTFILTERACTIONBASE_H_

#include <vector>
#include <memory>
#include <mutex>
#include "FilterActionBase.h"
#include "PostboxBase.h"


template <typename IOWrapper, typename Tin, typename Tout, template<typename, typename> typename L, typename LAlloc = std::allocator<Tin>>
class SingleInputFilterActionBase: public FilterActionBase<IOWrapper> {

	bool mBlockForOutput = false;

	std::shared_ptr<PostboxInputBase<Tout>> mOutput;
	std::shared_ptr<PostboxInputBase<Tout>> mOutputAwaited;//used only in processing thread, set only while holding mOutputMutex
	std::mutex mOutputMutex;

	std::shared_ptr<PostboxBase<Tin, L, LAlloc>> mInput;
protected:
	virtual Tout actOn(const L<Tin, LAlloc>& data) = 0;
public:

	SingleInputFilterActionBase(const std::shared_ptr<PostboxBase<Tin, L, LAlloc>>& input)

	bool action() override {
		if (!mInput->waitForPost()) return false;

		Tout newLatest = actOn(mInput->output());

		{
			std::lock_guard<std::mutex> lock(mOutputMutex);
			mOutputAwaited = mOutput;
		}

		if (mOutputAwaited != nullptr) {
			return mOutputAwaited->addDatum(newLatest, mBlockForOutput);// may block until datum read
		}
		return true;
	}

	std::size_t inputCount() override {
		return 1;
	}

	bool setOutput(typename IOWrapper::Wrapped&& wrappedInput) override {
		auto unwrapped = IOWrapper::template Unwrap<Tout>(std::move(wrappedInput));
		if (unwrapped != nullptr) {
			std::lock_guard<std::mutex> lock(mOutputMutex);
			if (mOutput != nullptr) mOutput->cancel();
			mOutput = unwrapped;
			return true;
		}
		return false;
	}

	typename IOWrapper::Wrapped getInput(int index, bool enableBlocking = true) const override {
		if (index != 0)return nullptr;
		if (mInput != nullptr) mInput->setEnableWait(enableBlocking);
		return IOWrapper::template Wrap<Tin>(mInput);
	}

	void setBlockForOutput(bool enable) override {
		mBlockForOutput = enable;
	}

	void cancel() override {
		mInput->cancel();
		{
			std::lock_guard<std::mutex> lock(mOutputMutex);
			if (mOutputAwaited != nullptr) mOutputAwaited->cancel();
		}
	}
};

/*template <typename Tin1,
	typename Tin2,
	typename Tout,
	template<typename, typename> typename L1,
	template<typename, typename> typename L2,
	typename LAlloc1 = std::allocator<Tin1>,
	typename LAlloc2 = std::allocator<Tin2> >
	class DoubleFilterActionBase {
	public:
		virtual ~DoubleFilterActionBase() = default;
		virtual Tout actOn(const L1<Tin1, LAlloc1>& data1, const L2<Tin2, LAlloc2>& data2) = 0;
};*/
#endif