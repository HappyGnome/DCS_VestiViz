#pragma once


#ifndef _MULTIPARTASYNCFILTER_H_
#define _MULTIPARTASYNCFILTER_H_

#include<thread>
#include<mutex>
#include <condition_variable>
#include <concurrent_vector.h>

#include "PostboxInputBase.h"
#include "AsyncFilter.h"
#include "FilterActionBase.h"
#include "ErrorStack.h"

template<typename IOWrapper>
class MultiPartAsyncFilter:public AsyncFilter<IOWrapper> {

	Concurrency::concurrent_vector<std::shared_ptr<FilterActionBase<IOWrapper>>> mFilterActions;
	std::vector< std::size_t > mLeafIndices;// index into mFilterActions
	std::vector< std::tuple<std::size_t, int> > mInputIndices; // (index into mFilterActions, input number)
	std::mutex mFilterIndicesMutex;
protected:

	bool process() override {
		for (auto it = mFilterActions.begin(); it != mFilterActions.end(); it++) {
			if(!(*it)->action())return false;
		}
		return true;
	}

	/**
	 * Cancel processing loop as soon as possible.
	 * Subsequent calls to waitForInput must return false and any blocking calls exit when possible.
	 */
	void cancel() override {
		for (auto it = mFilterActions.begin(); it != mFilterActions.end(); it++) {
			(*it)-> cancel();
		}
	}
public:
	static const std::size_t NEW_INPUT = -1;
	static const std::size_t LEAF_REJOINED = -1;

	explicit MultiPartAsyncFilter(std::shared_ptr<ErrorStack> log = nullptr) :AsyncFilter<IOWrapper>(log) {};

	typename IOWrapper::Wrapped getInput(int index, bool enableBlocking = true) override {
		std::lock_guard<std::mutex> lock(mFilterIndicesMutex);
		if (index >= mInputIndices.size() || index < 0) return nullptr;
		std::tuple<std::size_t, int>& inSpec = mInputIndices[index];

		return mFilterActions[std::get<0>(inSpec)]->getInput(std::get<1>(inSpec),enableBlocking);
	}

	bool addAction(std::shared_ptr<FilterActionBase<IOWrapper>> action,
		const std::vector<std::size_t>& fromLeaves,
		std::size_t& outputLeaf,
		std::vector<std::size_t>& outputNewInputHandles) {

		std::lock_guard<std::mutex> lock(mFilterIndicesMutex);
		if (action == nullptr || action->inputCount() > fromLeaves.size()) return false;

		//validate input connection params
		for (auto it = fromLeaves.cbegin(); it != fromLeaves.cend(); it++) {
			if(*it != NEW_INPUT 
				&& (mLeafIndices.size() <= *it || mLeafIndices[*it] == LEAF_REJOINED))return false;
		}

		//Try to make connections
		std::size_t inputNumber = 0;
		for (auto it = fromLeaves.cbegin(); it != fromLeaves.cend(); it++) {
			if(	*it != NEW_INPUT 
				&& !mFilterActions[mLeafIndices[*it]]->setOutput(action->getInput(inputNumber,false)))return false;
			inputNumber++;
		}

		mFilterActions.push_back(action);
		mLeafIndices.push_back(mFilterActions.size() - 1);
		outputLeaf = mLeafIndices.size() - 1;

		//validate input connection params
		inputNumber = 0;
		outputNewInputHandles.clear();
		for (auto it = fromLeaves.cbegin(); it != fromLeaves.cend(); it++) {
			if (*it != NEW_INPUT) mLeafIndices[*it] = LEAF_REJOINED;
			else {
				mInputIndices.emplace_back(mFilterActions.size() - 1, inputNumber);
				outputNewInputHandles.push_back(mInputIndices.size() - 1);
			}
			inputNumber++;
		}

		return true;
	}

	bool setOutput(typename IOWrapper::Wrapped&& wrappedInput, bool blockingOutput = false) override{
		std::lock_guard<std::mutex> lock(mFilterIndicesMutex);

		std::size_t leafFrom = 0;
		bool leafFound = false;
		for (auto it = mLeafIndices.cbegin(); it != mLeafIndices.cend(); it++) {
			if (*it != LEAF_REJOINED && leafFound) {
				 return false;
			}
			else if (*it == LEAF_REJOINED && !leafFound) {
				leafFrom++;
			}
			else leafFound = true;
			
		}
		if (!leafFound) return false;

		if (!mFilterActions[mLeafIndices[leafFrom]]->setOutput(std::move(wrappedInput))) return false;

		mFilterActions[mLeafIndices[leafFrom]]->setBlockForOutput(blockingOutput);

		mLeafIndices[leafFrom] = LEAF_REJOINED;

		return true;
	}


	int getInputCount() override{
		std::lock_guard<std::mutex> lock(mFilterIndicesMutex);

		return (int)mInputIndices.size();
	}

	bool validate() {
		std::lock_guard<std::mutex> lock(mFilterIndicesMutex);
		for (auto it = mLeafIndices.cbegin(); it != mLeafIndices.cend(); it++) {
			if (*it != LEAF_REJOINED) return false;
		}
		return true;
	}
};
#endif