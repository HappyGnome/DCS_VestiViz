#pragma once

#ifndef _EXPDECAYFILTERACTION_H_
#define _EXPDECAYFILTERACTION_H_

#include <vector>
#include <list>
#include <math.h>

#include"Datalin.h"
#include "TimedDatum.h"
#include "FilterActionBase.h"

template <typename S, typename T, typename L>
class ExpDecayFilterAction : public FilterActionBase<TimedDatum<S, T>, L> {
	S mHalflife;
	S mLastTime = 0;
	TimedDatum<S, T> mState= {0,0};
public:
	explicit ExpDecayFilterAction(S halflife) : mHalflife(halflife) {};

	TimedDatum<S, T> actOn(const L& data) override {
		for (auto it = data.cbegin(); it != data.cend(); it++) {
			S dt = it->t - mLastTime;

			S exponent = pow((S)0.5, dt / mHalflife);
		
			Datalin<S, T>::linEq(mState.datum,exponent, it->datum, 1.0 - exponent);
			mState.t = (mState.t * exponent) + (it->t * ((S)1.0 - exponent));

			mLastTime = it->t;
		}
		return mState;
	};
};

#endif // !_CONVOLVEFILTERACTION_H_

