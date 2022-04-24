#pragma once

#ifndef _LINCOMBFILTERACTION_H_
#define _LINCOMBFILTERACTION_H_

#include <vector>
#include <list>
#include <math.h>

#include"Datalin.h"
#include "TimedDatum.h"
#include "FilterActionBase.h"

template <
	typename S,
	typename T,
	template<typename, typename> typename L,
	typename LAlloc = std::allocator<T>>
class LinCombFilterAction : public DoubleFilterActionBase<
	TimedDatum<S, T>,
	TimedDatum<S, T>,
	TimedDatum<S, T>,
	L,
	L,
	LAlloc,
	LAlloc> {

	S mScaleX;
	S mScaleY;
	S mScaleTX;
	S mScaleTY;
public:
	explicit LinCombFilterAction(S scaleX, S scaleY) : mScaleX(scaleX), mScaleY(scaleY) {
		mScaleTX = std::abs(mScaleX);
		mScaleTY = std::abs(mScaleY);
		S sum = mScaleTX + mScaleTY;
		if (sum > 0) {
			mScaleTX /= sum;
			mScaleTY /= sum;
		}
		else {
			mScaleTX = (S)0.5;
			mScaleTY = (S)0.5;
		}
	};

	TimedDatum<S, T> actOn(
		const L<TimedDatum<S, T>,
		LAlloc>& vec1,
		const L<TimedDatum<S, T>,
		LAlloc>&vec2) override {

		TimedDatum<S, T> ret;

		if (!vec1.empty() && !vec2.empty()) {
			ret.datum = Datalin<S, T>::lin(vec1.crend()->datum,mScaleX, vec2.crend()->datum, mScaleY);
			ret.t = mScaleTX * vec1.crend()->t + mScaleTY * vec2.crend()->t;
		}		
		
		return ret; 
	};
};

#endif
