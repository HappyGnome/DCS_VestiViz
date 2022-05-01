#pragma once

#ifndef _EXPDECAYFILTERACTION_H_
#define _EXPDECAYFILTERACTION_H_

#include <vector>
#include <list>
#include <math.h>

#include"Datalin.h"
#include "TimedDatum.h"
#include "FilterActionBase.h"

template <typename S, typename T, template<typename, typename> typename L, typename LAlloc = std::allocator<TimedDatum<S, T>>>
class ExpDecayFilterAction : public FilterActionBase<TimedDatum<S, T>,TimedDatum<S, T>, L, LAlloc> {
	S mHalflife;
	S mLastTime = 0;
	S mNormalizationFactor = 0;
	TimedDatum<S, T> mState= TimedDatum<S, T>::zero();
public:
	explicit ExpDecayFilterAction(S halflife) : mHalflife(halflife) {};

	TimedDatum<S, T> actOn(const L<TimedDatum<S, T>, LAlloc>& data) override {
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
