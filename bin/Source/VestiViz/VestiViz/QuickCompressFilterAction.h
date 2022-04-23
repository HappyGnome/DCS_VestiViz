#pragma once

#ifndef _QUICKCOMPRESSFILTERACTION_H_
#define _QUICKCOMPRESSFILTERACTION_H_

#include <vector>
#include <list>
#include <math.h>

#include"Datalin.h"
#include "TimedDatum.h"
#include "FilterActionBase.h"

template <typename S, typename T, typename L>
class QuickCompressFilterAction : public FilterActionBase<TimedDatum<S, T>, L> {
	S mHalflife;
	S mLastTime = 0;
	S mNormalizationFactor = 0;
	TimedDatum<S, T> mState = { 0,0 };
public:
	explicit ExpDecayFilterAction(S halflife) : mHalflife(halflife) {};

	TimedDatum<S, T> actOn(const L& data) override {
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
			ret.datum *= 1 / mNormalizationFactor;
			ret.t /= mNormalizationFactor;
		}

		return ret;
	};
};

#endif // !_QUICKCOMPRESSFILTERACTION_H_

