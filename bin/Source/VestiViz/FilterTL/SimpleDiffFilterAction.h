#pragma once

#ifndef _SIMPLEDIFFFILTERACTION_H_
#define _SIMPLEDIFFFILTERACTION_H_

#include <vector>
#include <list>
#include <math.h>

#include"Datalin.h"
#include"Datacomp.h"
#include "TimedDatum.h"
#include "FilterActionBase.h"

template <typename S, typename T, template<typename, typename> typename L, typename LAlloc = std::allocator<TimedDatum<S, T>>>
class SimpleDiffFilterAction : public FilterActionBase<TimedDatum<S, T>, TimedDatum<S, T>, L, LAlloc> {
public:
	TimedDatum<S, T> actOn(const L<TimedDatum<S, T>, LAlloc>& data) override {
		if (data.size() < 2) return Datalin<S, TimedDatum<S, T>>::zero();

		auto pt2 = data.crbegin();
		auto pt1 = data.crbegin();
		pt1++;

		S dt = pt2->t - pt1->t;
		if (dt == 0) return Datalin<S, TimedDatum<S, T>>::zero();


		TimedDatum<S, T> ret;
		ret.t = (pt2->t + pt1->t) / 2;
		ret.datum = (pt2->datum - pt1->datum) * (1/dt);
		return ret;
	};
};

#endif // !_SIMPLEDIFFFILTERACTION_H_

