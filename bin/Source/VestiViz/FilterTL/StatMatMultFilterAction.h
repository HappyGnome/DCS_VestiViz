#pragma once

#ifndef _STATMATMULTFILTERACTION_H_
#define _STATMATMULTFILTERACTION_H_

#include <vector>
#include <list>
#include <math.h>

#include"Datalin.h"
#include "TimedDatum.h"
#include "SimplePostbox.h"
#include "FilterActionWithInputBase.h"

template <typename IOWrapper,
	typename S, 
	typename Tin, 
	typename Tmat,
	typename Tout> 
class StatMatMultFilterAction : public FilterActionWithInputBase<IOWrapper, TimedDatum<S, Tin>, TimedDatum<S, Tout>, CircBufL, std::allocator<TimedDatum<S, T>>> {
	Tmat mMat;

	using FAWIB = FilterActionWithInputBase<IOWrapper, TimedDatum<S, T>, TimedDatum<S, T>, CircBufL, std::allocator<TimedDatum<S, T>>>;
	using FAWIB::getInputData;
public:
	explicit StatMatMultFilterAction(Tmat&& mat) :
		FAWIB(std::shared_ptr<PostboxBase<TimedDatum<S, Tin>, CircBufL>>(new SimplePostbox< TimedDatum<S, Tin>>()))
		mMat(std::move(mat)) {};

	TimedDatum<S, Tout> actOn() override {
		CircBufL<TimedDatum<S, Tin>> vec;
		getInputData<CircBufL<TimedDatum<S, Tin>>, 0>(vec);

		TimedDatum<S, Tout> ret;

		if (!vec.empty()) {
			ret.datum = mMat.applyTo(vec.crbegin() -> datum);
			ret.t = vec.crbegin()->t;
		}
		return ret;
	};
};

#endif

