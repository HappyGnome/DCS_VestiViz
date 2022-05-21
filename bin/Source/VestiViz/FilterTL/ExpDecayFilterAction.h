#pragma once

#ifndef _EXPDECAYFILTERACTION_H_
#define _EXPDECAYFILTERACTION_H_

#include <vector>
#include <list>
#include <math.h>

#include"Datalin.h"
#include "SimplePostbox.h"
#include "TimedDatum.h"
#include "FilterActionWithInputBase.h"

template <typename IOWrapper, typename S, typename T>
class ExpDecayFilterAction : public FilterActionWithInputBase<IOWrapper, TimedDatum<S, T>, CircBufL, std::allocator,TimedDatum<S, T>> {
	S mHalflife;
	S mLastTime = 0;
	S mNormalizationFactor = 0;
	TimedDatum<S, T> mState= TimedDatum<S, T>::zero();

	using FAWIB = FilterActionWithInputBase<IOWrapper, TimedDatum<S, T>, CircBufL, std::allocator, TimedDatum<S, T>>;
	using FAWIB::getInputData;
public:
	explicit ExpDecayFilterAction(S halflife) : 
		FAWIB(std::shared_ptr<PostboxBase<TimedDatum<S, T>, CircBufL>>(new SimplePostbox< TimedDatum<S, T>>())),
		mHalflife(halflife) {};

	TimedDatum<S, T> actOn() override {

		CircBufL<TimedDatum<S, T>> data;
		getInputData<CircBufL<TimedDatum<S, T>>, 0>(data);

		for (auto it = data.cbegin(); it != data.cend(); it++) {
			S dt = it->t - mLastTime;

			S exponent = pow((S)0.5, dt / mHalflife);
			S conjExponent = 1 - exponent;
			
			mNormalizationFactor = mNormalizationFactor * exponent + conjExponent;

			Datalin<S, T>::linEq(mState.datum, exponent, it->datum, conjExponent);
			mState.t = (mState.t * exponent) + (it->t * conjExponent);

			mLastTime = it->t;
		}

		TimedDatum<S, T> ret = mState;

		if (mNormalizationFactor > 0) {
			ret.datum *= 1/mNormalizationFactor;
			ret.t /= mNormalizationFactor;
		}

		return ret;
	};
};

#endif
