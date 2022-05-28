#pragma once

#ifndef _FILTERACTIONWITHINPUTBASE_H_
#define _FILTERACTIONWITHINPUTBASE_H_

#include <vector>
#include <memory>
#include <mutex>
#include <concurrent_vector.h>
#include "FilterActionBase.h"
#include "PostboxBase.h"


template <typename IOWrapper, typename Tout, template<typename, typename> typename L, template<typename> typename LAlloc, typename Tin, typename ...Args>
class FilterActionWithInputBase : public FilterActionWithInputBase<IOWrapper,Tout, L, LAlloc, Args...> {
	std::shared_ptr<PostboxBase<Tin, L, LAlloc<Tin>>> mInput;
protected:
	template<typename T, std::size_t N> 
	bool getInputData(T& dataOut) {
		return typename FilterActionWithInputBase<IOWrapper, Tout, L, LAlloc, Args...>::template getInputData<T, N - 1>(dataOut);
	}
	template<>
	bool getInputData<L<Tin,LAlloc<Tin>>,0>(L<Tin, LAlloc<Tin>>& dataOut) {
		dataOut = mInput->output();
		return true;
	}

	bool waitForPost() {
		return mInput->waitForPost() && FilterActionWithInputBase<IOWrapper, Tout, L, LAlloc, Args...>::waitForPost();
	}

	using FilterActionWithInputBase<IOWrapper, Tout, L, LAlloc, Args...>::actOnAndPost;
public:
	explicit FilterActionWithInputBase(const std::shared_ptr<PostboxBase<Tin, L, LAlloc<Tin>>>& input, const std::shared_ptr<PostboxBase<Args, L, LAlloc<Args>>>& ... rest) :
		FilterActionWithInputBase <IOWrapper, Tout, L, LAlloc, Args...>(rest...), mInput(input) {}

	bool action() override {
		if (!waitForPost()) return false;

		return actOnAndPost();
	}

	constexpr std::size_t inputCount() override {
		return 1 + FilterActionWithInputBase <IOWrapper, Tout, L, LAlloc, Args...>::inputCount();
	}

	typename IOWrapper::Wrapped getInput(int index, bool enableBlocking = true) const override {
		if (index < 0)return nullptr;
		else if (index > 0) return FilterActionWithInputBase <IOWrapper, Tout, L, LAlloc, Args...>::getInput(index - 1, enableBlocking);
		else if (mInput != nullptr) mInput->setEnableWait(enableBlocking);
		return IOWrapper::template Wrap<Tin>(mInput);
	}

	void cancel() override {
		mInput->cancel();
		FilterActionWithInputBase <IOWrapper, Tout, L, LAlloc, Args...>::cancel();
	}
};


// Recursion base -------------------------------------------------------------------------------------------------------------

template <typename IOWrapper, typename Tout, 
	template<typename, typename> typename L, template<typename> typename LAlloc, typename Tin>
class FilterActionWithInputBase<IOWrapper, Tout, L, LAlloc, Tin> : public FilterActionBase<IOWrapper> {

	bool mBlockForOutput = false;

	Concurrency::concurrent_vector<std::shared_ptr<PostboxInputBase<Tout>>> mOutputs;

	std::shared_ptr<PostboxBase<Tin, L, LAlloc<Tin>>> mInput;
protected:
	virtual Tout actOn() = 0;

	template<typename T, std::size_t N>
	bool getInputData(T& dataOut) {
		return false;
	}

	template<>
	bool getInputData<L<Tin, LAlloc<Tin>>, 0>(L<Tin, LAlloc<Tin>>& dataOut) {
		dataOut = mInput->output();
		return true;
	}

	bool waitForPost() {
		return mInput->waitForPost();
	}

	bool actOnAndPost() {
		Tout newLatest = actOn();

		for (auto it = mOutputs.begin(); it != mOutputs.end(); it++) {
			if (*it != nullptr) {
				(*it)->addDatum(newLatest, mBlockForOutput);// may block until datum read
			}
		}
		return true;
	}

public:
	explicit FilterActionWithInputBase(const std::shared_ptr<PostboxBase<Tin, L, LAlloc<Tin>>>& input) : mInput(input), mOutputs(1) {}

	bool action() override {
		if (!waitForPost()) return false;

		return actOnAndPost();
	}

	constexpr std::size_t inputCount() override {
		return 0;
	}
	typename IOWrapper::Wrapped getInput(int index, bool enableBlocking = true) const override { 
		if (index != 0)return nullptr;
		else return IOWrapper::template Wrap<Tin>(mInput);
	}

	bool setOutput(typename IOWrapper::Wrapped&& wrappedInput) override {
		auto unwrapped = IOWrapper::template Unwrap<Tout>(std::move(wrappedInput));

		if (unwrapped != nullptr) {
			for (int i = 0; i < mOutputs.size(); i++) {
				if (mOutputs[i] == nullptr) {
					mOutputs[i] = unwrapped;
					return true;
				}				
			}
		}
		return false;
	}
	int addOutput() override {		
		mOutputs.push_back(nullptr);
		return mOutputs.size() - 1;
	}
	bool allOutputsJoined() override {
		for (auto it = mOutputs.cbegin(); it != mOutputs.cend(); it++) {
			if(*it == nullptr) return false;
		}
		return true;
	}

	void setBlockForOutput(bool enable) override {
		mBlockForOutput = enable;
	}

	void cancel() override {
		mInput->cancel();
		for (auto it = mOutputs.begin(); it != mOutputs.end(); it++)
		{
			if (*it != nullptr) (*it)->cancel();
		}
	}
};

#endif