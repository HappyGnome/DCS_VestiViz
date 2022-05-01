#pragma once

#ifndef _DYNMATMULTFILTERACTION_H_
#define _DYNMATMULTFILTERACTION_H_

#include <vector>
#include <list>
#include <math.h>

#include"Datalin.h"
#include "TimedDatum.h"
#include "FilterActionBase.h"

template <
	typename S,
	typename Tout,
	typename Tin1,
	typename Tin2,
	template<typename, typename> typename L1,
	template<typename, typename> typename L2,
	typename LAlloc1 = std::allocator<TimedDatum<S, Tin1>>,
	typename LAlloc2 = std::allocator<TimedDatum<S, Tin2>> >
class DynMatMultFilterAction : public DoubleFilterActionBase<
			TimedDatum<S, Tin1>, 
			TimedDatum<S, Tin2>,
			TimedDatum<S, Tout>,
			L1,
			L2,
			LAlloc1,
			LAlloc2> {
public:
	TimedDatum<S, Tout> actOn(
		const L1<TimedDatum<S, Tin1>,
		LAlloc1>& vec,
		const L2<TimedDatum<S, Tin2>,
		LAlloc2>& matrix) override {
		
		TimedDatum<S, Tout> ret;

		if (!vec.empty() && !matrix.empty()) {
			ret.datum = matrix.crbegin()->datum.applyTo(vec.crbegin() -> datum);
			ret.t = (S)0.5 * (matrix.crbegin()->t + vec.crbegin()->t);
		}
		return ret;
	};
};

#endif
