#pragma once

#ifndef _LINCOMBFILTERACTION_H_
#define _LINCOMBFILTERACTION_H_

#include <vector>
#include <list>
#include <math.h>

#include"Datalin.h"
#include "SimplePostbox.h"
#include "TimedDatum.h"
#include "FilterActionWithInputBase.h"

template <
	typename S,
	typename T>
class LinCombFilterAction : public FilterActionWithInputBase<
									TimedDatum<S, T>,
									TimedDatum<S, T>,	
									CircBufL, std::allocator<TimedDatum<S, T>>,
									TimedDatum<S, T>,
									CircBufL, std::allocator<TimedDatum<S, T>>> {

	S mScaleX;
	S mScaleY;
	S mScaleTX;
	S mScaleTY;

	using FAWIB = FilterActionWithInputBase<
		TimedDatum<S, T>,
		TimedDatum<S, T>,
		CircBufL, std::allocator<TimedDatum<S, T>>,
		TimedDatum<S, T>,
		CircBufL, std::allocator<TimedDatum<S, T>>>;
	using FAWIB::getInputData;
public:
	LinCombFilterAction() = default;

	explicit LinCombFilterAction(S scaleX, S scaleY) :
		FAWIB(std::shared_ptr<PostboxBase<TimedDatum<S, T>, CircBufL>>(new SimplePostbox< TimedDatum<S, T>>())),
		mScaleX(scaleX),
		mScaleY(scaleY) {

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

	TimedDatum<S, T> actOn() override {

		CircBufL<TimedDatum<S, T>> vec1;
		CircBufL<TimedDatum<S, T>> vec2;

		getInputData<CircBufL<TimedDatum<S, T>>, 0>(vec1);
		getInputData<CircBufL<TimedDatum<S, T>>, 1>(vec2);

		TimedDatum<S, T> ret;

		if (!vec1.empty() && !vec2.empty()) {
			ret.datum = Datalin<S, T>::lin(vec1.crbegin()->datum,mScaleX, vec2.crbegin()->datum, mScaleY);
			ret.t = mScaleTX * vec1.crbegin()->t + mScaleTY * vec2.crbegin()->t;
		}		
		
		return ret; 
	};
};

#endif
