#pragma once
#ifndef _PIPELINEBASE_H_
#define _PIPELINEBASE_H_

#include<vector>
#include<memory>

#include "AsyncFilter.h"

#include "SingleInputFilterBase.h"
#include "DoubleInputFilterBase.h"
#include "OutputFilterBase.h"
#include"FilterActionBase.h"
#include "CircBuf.h"
#include"TimedDatum.h"
#include"SimplePostbox.h"

template<typename IOWrapper>
class PipelineBase {
	std::vector< std::shared_ptr<AsyncFilter<IOWrapper>>> mFilters;
	std::vector< std::size_t > mLeafIndices;// index into mFilters
	std::vector< std::tuple<std::size_t, int> > mInputIndices; // (index into mFilters, input number)
	std::shared_ptr<ErrorStack> mErrors;

	bool TryAddSIF(std::size_t fromLeaf, std::shared_ptr<AsyncFilter<IOWrapper>> newFilter, std::size_t& outputLeaf) {
		if (fromLeaf == NEW_INPUT) {
			mFilters.push_back(newFilter);
			mInputIndices.emplace_back(mFilters.size() - 1, 0);
			mLeafIndices.push_back(mFilters.size() - 1);
			outputLeaf = mLeafIndices.size() - 1;
			return true;
		}
		else if (mLeafIndices.size() > fromLeaf && mLeafIndices[fromLeaf] != LEAF_REJOINED 
			&& mFilters[mLeafIndices[fromLeaf]]->setOutput(newFilter->getInput(0))) {

			mFilters.push_back(newFilter);
			mLeafIndices[fromLeaf] = mFilters.size() - 1;
			outputLeaf = fromLeaf;
			return true;
		}
		else return false;
	}
	bool TryAddDIF(std::size_t fromLeaf1,
		std::size_t fromLeaf2,
		std::shared_ptr<AsyncFilter<IOWrapper>> newFilter,
		std::size_t& outputLeaf) {

		if ((fromLeaf1 != NEW_INPUT && (mLeafIndices.size() <= fromLeaf1 || mLeafIndices[fromLeaf1] == LEAF_REJOINED))
			|| (fromLeaf2 != NEW_INPUT && (mLeafIndices.size() <= fromLeaf2 || mLeafIndices[fromLeaf2] == LEAF_REJOINED)))
			return false;

		if (fromLeaf1 != NEW_INPUT && !mFilters[mLeafIndices[fromLeaf1]]->setOutput(newFilter->getInput(0))) return false;
		if (fromLeaf2 != NEW_INPUT && !mFilters[mLeafIndices[fromLeaf2]]->setOutput(newFilter->getInput(1))) {
			if (fromLeaf1 != NEW_INPUT)mFilters[mLeafIndices[fromLeaf1]]->unsetOutput();
			return false;
		}

		mFilters.push_back(newFilter);
		mLeafIndices.push_back(mFilters.size() - 1);
		outputLeaf = mLeafIndices.size() - 1;
		if (fromLeaf1 != NEW_INPUT) mLeafIndices[fromLeaf1] = LEAF_REJOINED;
		else mInputIndices.emplace_back(mFilters.size() - 1, 0);

		if (fromLeaf2 != NEW_INPUT) mLeafIndices[fromLeaf2] = LEAF_REJOINED;
		else mInputIndices.emplace_back(mFilters.size() - 1, 1);

		return true;
	}

public:
	static const std::size_t NEW_INPUT = -1;
	static const std::size_t LEAF_REJOINED = -1;

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

	template<typename S, typename Tin, typename Tout>
	using PFAB = std::unique_ptr< FilterActionBase<TimedDatum<S, Tin>, TimedDatum<S, Tout>, CircBufL>>;

	template<typename S, typename Tin1, typename Tin2, typename Tout>
	using PDFAB = std::unique_ptr< DoubleFilterActionBase<TimedDatum<S, Tin1>, TimedDatum<S, Tin2>, TimedDatum<S, Tout>, CircBufL, CircBufL>>;

	template<typename S, typename Tin, typename Tout>
	bool addBufferedSIF(std::size_t fromLeaf,
		std::size_t bufferSize,
		PFAB<S,Tin,Tout>&& action,
		std::size_t& outputLeaf) {

		auto newFilter = std::shared_ptr<AsyncFilter<IOWrapper>>(new SingleInputFilterBase<TimedDatum<S, Tin>, TimedDatum<S, Tout>, IOWrapper, CircBufL>
			(std::make_shared<CircPostbox<TimedDatum<S, Tin>>>(bufferSize), std::move(action), mErrors));

		return TryAddSIF(fromLeaf, newFilter, outputLeaf);
	}

	template<typename S, typename Tin, typename Tout>
	bool addSimpleSIF(std::size_t fromLeaf,
		PFAB<S, Tin, Tout>&& action,
		std::size_t& outputLeaf) {

		auto newFilter = std::shared_ptr<AsyncFilter<IOWrapper>>(new SingleInputFilterBase<TimedDatum<S, Tin>, TimedDatum<S, Tout>, IOWrapper, CircBufL>
			(std::make_shared<SimplePostbox<TimedDatum<S, Tin>>>(), std::move(action), mErrors));

		return TryAddSIF(fromLeaf, newFilter, outputLeaf);
	}

	template<typename S, typename Tin1, typename Tin2, typename Tout>
	bool addSimpleDIF(std::size_t fromLeaf1, std::size_t fromLeaf2,
		PDFAB<S, Tin1, Tin2, Tout>&& action,
		std::size_t& outputLeaf) {

		auto newFilter = std::shared_ptr<AsyncFilter<IOWrapper>>(new DoubleInputFilterBase<
			TimedDatum<S, Tin1>, 
			TimedDatum<S, Tin2>,
			TimedDatum<S, Tout>, 
			IOWrapper, 
			CircBufL,
			CircBufL>
			(std::make_shared<SimplePostbox<TimedDatum<S, Tin1>>>(),
			 std::make_shared<SimplePostbox<TimedDatum<S, Tin2>>>(), 
			 std::move(action), 
			 mErrors));

		return TryAddDIF(fromLeaf1, fromLeaf2, newFilter, outputLeaf);
	}

	template<typename S, typename Tin, typename Tout>
	bool addBufferedOutF(std::size_t fromLeaf,
		std::size_t bufferSize,
		PFAB<S, Tin, Tout>&& action,
		std::size_t& outputLeaf) {

		auto newFilter = std::shared_ptr<AsyncFilter<IOWrapper>>(new OutputFilterBase<
			TimedDatum<S, Tin>, 
			TimedDatum<S, Tout>,
			IOWrapper, 
			CircBufL>
			(std::make_shared<CircPostbox<TimedDatum<S, Tin>>>(bufferSize), std::move(action), mErrors));

		return TryAddSIF(fromLeaf, newFilter, outputLeaf);
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
		return true;
	}
};

#endif