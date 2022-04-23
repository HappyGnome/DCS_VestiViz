#pragma once

#ifndef _CONVOLVEFILTERACTION_H_
#define _CONVOLVEFILTERACTION_H_

#include <vector>
#include<list>

#include "TimedDatum.h"
#include "FilterActionBase.h"
#include "Datalin.h"

template <typename S, typename T, template<typename,typename> typename L, typename LAlloc = std::allocator<TimedDatum<S, T>>>
class ConvolveFilterAction : public FilterActionBase<TimedDatum<S, T>,TimedDatum<S,T>,L,LAlloc>{
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
public:
	explicit ConvolveFilterAction(std::vector<S>&& kernel) :mKernel(kernel){
		makeTimeKernel();
	}

	TimedDatum<S, T> actOn(const L<TimedDatum<S, T>,LAlloc>& data) override {
		std::size_t window = std::min(mKernel.size(), data.size());
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

