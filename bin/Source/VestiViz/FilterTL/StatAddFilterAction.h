#pragma once

#ifndef _STATADDFILTERACTION_H_
#define _STATADDFILTERACTION_H_

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
	typename LAlloc = std::allocator<TimedDatum<S, T>>>
	class StatAddFilterAction : public FilterActionBase<TimedDatum<S, T>, TimedDatum<S, T>, L, LAlloc> {
	T mAdd;
	public:
		explicit StatAddFilterAction(T&& add) : mAdd(std::move(add)) {};

		TimedDatum<S, T> actOn(
			const L<TimedDatum<S, T>,LAlloc>& vec) override {

			TimedDatum<S, T> ret;

			if (!vec.empty()) {
				ret.datum = vec.crbegin()->datum + mAdd;
				ret.t = vec.crbegin()->t;
			}
			return ret;
		};
};

#endif

