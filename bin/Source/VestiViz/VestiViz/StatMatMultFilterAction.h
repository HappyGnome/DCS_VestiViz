#pragma once

#ifndef _STATMATMULTFILTERACTION_H_
#define _STATMATMULTFILTERACTION_H_

#include <vector>
#include <list>
#include <math.h>

#include"Datalin.h"
#include "TimedDatum.h"
#include "FilterActionBase.h"

template <
	typename S, 
	typename Tin, 
	typename Tmat,
	typename Tout,
	template<typename, typename> typename L, 
	typename LAlloc = std::allocator<TimedDatum<S, Tin>>> 
class StatMatMultFilterAction : public FilterActionBase<TimedDatum<S, Tin>, TimedDatum<S, Tout>, L, LAlloc> {
	Tmat mMat;
public:
	explicit StatMatMultFilterAction(Tmat mat) : mMat(mat) {};

	TimedDatum<S, Tout> actOn(
		const L1<TimedDatum<S, Tin>,
		LAlloc1>& vec) override {

		TimedDatum<S, Tout> ret;

		if (!vec.empty()) {
			ret.datum = mMat.applyTo(*vec.crend());
			ret.t = vec.crend()->t;
		}
		return ret;
	};
};

#endif

