#pragma once

#ifndef _SIMPLEDIFFFILTERACTION_H_
#define _SIMPLEDIFFFILTERACTION_H_

#include <vector>
#include <list>
#include <math.h>

#include"Datalin.h"
#include"Datacomp.h"
#include "TimedDatum.h"
#include "CircPostbox.h"
#include "FilterActionWithInputBase.h"

template <typename IOWrapper, typename S, typename T>
class SimpleDiffFilterAction : public FilterActionWithInputBase<IOWrapper, TimedDatum<S, T>, TimedDatum<S, T>, CircBufL, std::allocator<TimedDatum<S, T>>> {

	using FAWIB = FilterActionWithInputBase<IOWrapper, TimedDatum<S, T>, TimedDatum<S, T>, CircBufL, std::allocator<TimedDatum<S, T>>>;
	using FAWIB::getInputData;
public:

	explicit SimpleDiffFilterAction() : FAWIB(std::shared_ptr<PostboxBase<TimedDatum<S, T>, CircBufL>>(new CircPostbox< TimedDatum<S, T>>(2))) {}

	TimedDatum<S, T> actOn() override {

		CircBufL<TimedDatum<S, T>> data;
		getInputData<CircBufL<TimedDatum<S, T>>, 0>(data);

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

