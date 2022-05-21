#pragma once
#ifndef _PIPELINEBASE_H_
#define _PIPELINEBASE_H_

#include<vector>
#include<memory>

#include "MultiPartAsyncFilter.h"
#include"FilterActionWithInputBase.h"
#include "CircBuf.h"
#include"TimedDatum.h"
#include"SimplePostbox.h"

template<typename IOWrapper>
class PipelineBase {
	std::vector< std::shared_ptr<MultiPartAsyncFilter<IOWrapper>>> mFilters;
	std::vector< std::size_t> mLeafIndices;// index into mFilters
	std::vector< std::size_t> mLeavesInConstructionIndices;// index into mFilters
	std::vector< std::tuple<std::size_t, int> > mInputIndices; // (index into mFilters, input number)
	std::shared_ptr<ErrorStack> mErrors;
protected:
	bool TryConnectFilter(const std::vector<std::size_t>& fromLeaves, std::size_t leafToAddIndex, std::size_t& outputLeaf, std::vector<std::size_t>& newInputs) {

		if (leafToAddIndex >= mLeavesInConstructionIndices.size())return false;
		std::size_t filterIndex = mLeavesInConstructionIndices[leafToAddIndex];
		std::shared_ptr<MultiPartAsyncFilter<IOWrapper>> filterToConnect = mFilters[filterIndex];
		if(filterIndex == LEAF_CONSTRUCTED || filterIndex > mFilters.size() || fromLeaves.size() < filterToConnect->getInputCount()) return false;

		//validate input connection params
		for (auto it = fromLeaves.cbegin(); it != fromLeaves.cend(); it++) {
			if (*it != NEW_INPUT
				&& (mLeafIndices.size() <= *it 
					|| mLeafIndices[*it] == LEAF_REJOINED)) return false;
		}

		//Try to make connections
		std::size_t inputNumber = 0;
		for (auto it = fromLeaves.cbegin(); it != fromLeaves.cend(); it++) {
			if (*it != NEW_INPUT
				&& !mFilters[mLeafIndices[*it]]->setOutput(filterToConnect->getInput(inputNumber, true)))return false;
			inputNumber++;
		}
		mLeavesInConstructionIndices[leafToAddIndex] = LEAF_CONSTRUCTED;
		mLeafIndices.push_back(filterIndex);
		outputLeaf = mLeafIndices.size() - 1;

		//validate input connection params
		inputNumber = 0;
		newInputs.clear();
		for (auto it = fromLeaves.cbegin(); it != fromLeaves.cend(); it++) {
			if (*it != NEW_INPUT) mLeafIndices[*it] = LEAF_REJOINED;
			else {
				mInputIndices.emplace_back(mFilters.size() - 1, inputNumber);
				newInputs.push_back(mInputIndices.size() - 1);
			}
			inputNumber++;
		}

		return true;
	}


	bool TryAddFilterAction(std::shared_ptr<FilterActionBase<IOWrapper>> newAction, 		
		std::size_t leafToAddIndex, 
		const std::vector<std::size_t>& fromInternalLeaves,
		std::size_t& outputLeaf, 
		std::size_t& outputActionLeaf,
		std::vector<std::size_t>& outputActionNewInputs) {

		if (newAction == nullptr) return false;
		if (leafToAddIndex != NEW_LEAF_FILTER && leafToAddIndex >= mLeavesInConstructionIndices.size()) return false;

		std::size_t filterIndex;
		if (leafToAddIndex != NEW_LEAF_FILTER) {
			filterIndex = mLeavesInConstructionIndices[leafToAddIndex];
			outputLeaf = leafToAddIndex;
		}
		else {
			
			mFilters.push_back(std::shared_ptr<MultiPartAsyncFilter<IOWrapper>>(new MultiPartAsyncFilter<IOWrapper>()));
			filterIndex = mFilters.size() - 1;
			mLeavesInConstructionIndices.push_back(filterIndex);
			outputLeaf = mLeavesInConstructionIndices.size() - 1;
		}

		if (mFilters[filterIndex] == nullptr) return false;
		return mFilters[filterIndex]->addAction(newAction, fromInternalLeaves, outputActionLeaf, outputActionNewInputs);
	}

	bool TryGetFilterInputCount(std::size_t filterInConstructionHandle, int& output) {
		if (filterInConstructionHandle >= mLeavesInConstructionIndices.size())return false;
		std::size_t filterIndex = mLeavesInConstructionIndices[filterInConstructionHandle];
		output = mFilters[filterIndex]->getInputCount();
		return true;
	}

public:
	static const std::size_t NEW_INPUT = -1;
	static const std::size_t LEAF_REJOINED = -1;
	static const std::size_t LEAF_CONSTRUCTED = -1;
	static const std::size_t NEW_LEAF_FILTER = -1;

	explicit PipelineBase(std::shared_ptr<ErrorStack> log = nullptr) :mErrors(log) {}

	void startPipeline() {
		for (auto it = mFilters.begin(); it != mFilters.end(); it++)
		{
			(*it)->startProcessing();
		}
	}
	void stopPipeline() {
		for (auto it = mFilters.begin(); it != mFilters.end(); it++)
		{
			(*it)->stopProcessing();
		}
	}

	typename IOWrapper::Wrapped getInput(std::size_t index) {
		if (index >= mInputIndices.size()) return nullptr;
		std::tuple<std::size_t,int> &inSpec = mInputIndices[index];

		return mFilters[std::get<0>(inSpec)]->getInput(std::get<1>(inSpec));
	}

	typename IOWrapper::Wrapped  getLastInput(int offset = 0) {
		if (mInputIndices.size() <= offset) return nullptr;
		std::tuple<std::size_t, int>& inSpec = mInputIndices[mInputIndices.size() - offset -1];

		return mFilters[std::get<0>(inSpec)]->getInput(std::get<1>(inSpec));
	}

	bool setOutput(std::size_t leafFrom, typename IOWrapper::Wrapped&& wrappedInput) {
		if (leafFrom >= mLeafIndices.size() || mLeafIndices[leafFrom] == LEAF_REJOINED) return false;
		if(!mFilters[mLeafIndices[leafFrom]]->setOutput(std::move(wrappedInput))) return false;

		mLeafIndices[leafFrom] = LEAF_REJOINED;

		return true;
	}

	bool validate() {
		for (auto it = mLeafIndices.cbegin(); it != mLeafIndices.cend(); it++) {
			if (*it != LEAF_REJOINED) return false;
		}

		for (auto it = mFilters.cbegin(); it != mFilters.cend(); it++) {
			if (!(*it) -> validate()) return false;
		}
		return true;
	}
};

#endif