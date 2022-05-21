#pragma once

#ifndef _CONVOLVEFILTERACTION_H_
#define _CONVOLVEFILTERACTION_H_

#include <vector>
#include <list>
#include <algorithm>

#include "TimedDatum.h"
#include "CircPostbox.h"
#include "FilterActionWithInputBase.h"
#include "Datalin.h"

template <typename IOWrapper,typename S, typename T>
class ConvolveFilterAction : public FilterActionWithInputBase<IOWrapper, TimedDatum<S, T>, CircBufL, std::allocator, TimedDatum<S,T>>{
	std::vector<S> mKernel;
	std::vector<S> mTimeKernel;

	void makeTimeKernel() {
		S absSum = 0;
		for (auto it = mKernel.cbegin(); it != mKernel.cend(); it++) {
			absSum += std::abs(*it);
		}
		if (absSum > 0)
		{
			for (auto it = mKernel.cbegin(); it != mKernel.cend(); it++) {
				mTimeKernel.push_back(std::abs(*it) / absSum);
			}
		}
		else mTimeKernel = mKernel;//both are zero
	}

	using FAWIB = FilterActionWithInputBase<IOWrapper, TimedDatum<S, T>, CircBufL, std::allocator , TimedDatum<S, T>>;
	using FAWIB::getInputData;
public:
	explicit ConvolveFilterAction(std::vector<S>&& kernel) :FAWIB(std::shared_ptr<PostboxBase<TimedDatum<S, T>, CircBufL>>(new CircPostbox< TimedDatum<S, T>>(kernel.size()))), mKernel(kernel){
		makeTimeKernel();
	}

	TimedDatum<S, T> actOn() override {
		CircBufL<TimedDatum<S, T>> data;
		getInputData<CircBufL<TimedDatum<S, T>>, 0>(data);

		std::size_t window = std::min<std::size_t>(mKernel.size(), data.size());
		auto itK = mKernel.cbegin();
		auto itT = mTimeKernel.cbegin();
		auto itD = data.crbegin();
		TimedDatum<S, T> result = TimedDatum<S, T>();

		for (std::size_t i = 0; i < window; i++) {
			Datalin<S,T>::linEq(result.datum,itD->datum,*itK);
			result.t += itD->t * *itT;
			itK++;
			itD++;
		}

		return result;
	};
};

#endif // !_CONVOLVEFILTERACTION_H_

