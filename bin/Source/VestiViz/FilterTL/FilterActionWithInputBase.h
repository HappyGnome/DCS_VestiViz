#pragma once

#ifndef _FILTERACTIONWITHINPUTBASE_H_
#define _FILTERACTIONWITHINPUTBASE_H_

#include <vector>
#include <memory>
#include <mutex>
#include "FilterActionBase.h"
#include "PostboxBase.h"


template <typename IOWrapper, typename Tout, typename Tin, template<typename, typename> typename L, typename LAlloc, typename ...Args>
class FilterActionWithInputBase : public FilterActionWithInputBase<IOWrapper,Tout, Args...> {
	std::shared_ptr<PostboxBase<Tin, L, LAlloc>> mInput;
protected:
	template<typename T, std::size_t N> 
	bool getInputData(T& dataOut) {
		return typename FilterActionWithInputBase<IOWrapper, Tout, Args...>::template getInputData<T, N - 1>(dataOut);
	}
	template<>
	bool getInputData<L<Tin,LAlloc>,0>(L<Tin, LAlloc>& dataOut) {
		dataOut = mInput->output();
		return true;
	}

	bool waitForPost() {
		return mInput->waitForPost() && FilterActionWithInputBase<IOWrapper, Tout, Args...>::waitForPost();
	}

	using FilterActionWithInputBase<IOWrapper, Tout, Args...>::actOnAndPost;
public:
	explicit FilterActionWithInputBase(const std::shared_ptr<PostboxBase<Tin, L, LAlloc>>& input, Args... rest) : FilterActionWithInputBase <IOWrapper, Tout, Args...>(rest...), mInput(input) {}

	bool action() override {
		if (!waitForPost()) return false;

		return actOnAndPost();
	}

	constexpr std::size_t inputCount() override {
		return 1 + FilterActionWithInputBase <IOWrapper, Tout, Args...>::inputCount();
	}

	typename IOWrapper::Wrapped getInput(int index, bool enableBlocking = true) const override {
		if (index < 0)return nullptr;
		else if (index > 0) return FilterActionWithInputBase <IOWrapper, Tout, Args...>::getInput(index, enableBlocking);
		else if (mInput != nullptr) mInput->setEnableWait(enableBlocking);
		return IOWrapper::template Wrap<Tin>(mInput);
	}
};


// Recursion base -------------------------------------------------------------------------------------------------------------

template <typename IOWrapper, typename Tout, typename Tin, template<typename, typename> typename L, typename LAlloc>
class FilterActionWithInputBase : public FilterActionBase<IOWrapper> {

	bool mBlockForOutput = false;

	std::shared_ptr<PostboxInputBase<Tout>> mOutput;
	std::shared_ptr<PostboxInputBase<Tout>> mOutputAwaited;//used only in processing thread, set only while holding mOutputMutex
	std::mutex mOutputMutex;

	std::shared_ptr<PostboxBase<Tin, L, LAlloc>> mInput;
protected:
	virtual Tout actOn() = 0;

	template<typename T, std::size_t N>
	bool getInputData(T& dataOut) {
		return false;
	}

	template<>
	bool getInputData<L<Tin, LAlloc>, 0>(L<Tin, LAlloc>& dataOut) {
		dataOut = mInput->output();
		return true;
	}

	bool waitForPost() {
		return mInput->waitForPost();
	}

	bool actOnAndPost() {
		Tout newLatest = actOn();

		{
			std::lock_guard<std::mutex> lock(mOutputMutex);
			mOutputAwaited = mOutput;
		}

		if (mOutputAwaited != nullptr) {
			return mOutputAwaited->addDatum(newLatest, mBlockForOutput);// may block until datum read
		}
		return true;
	}

public:
	explicit FilterActionWithInputBase(const std::shared_ptr<PostboxBase<Tin, L, LAlloc>>& input) : mInput(input) {}

	bool action() override {
		if (!waitForPost()) return false;

		return actOnAndPost();
	}

	constexpr std::size_t inputCount() override {
		return 0;
	}
	typename IOWrapper::Wrapped getInput(int index, bool enableBlocking = true) const override { return nullptr; }

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

#endif