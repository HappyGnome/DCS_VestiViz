#pragma once

#ifndef _STATADDFILTERACTION_H_
#define _STATADDFILTERACTION_H_

#include <vector>
#include <list>
#include <math.h>

#include"Datalin.h"
#include "SimplePostbox.h"
#include "TimedDatum.h"
#include "FilterActionWithInputBase.h"

template <
	typename IOWrapper,
	typename S,
	typename T>
class StatAddFilterAction : public FilterActionWithInputBase<IOWrapper, TimedDatum<S, T>, CircBufL, std::allocator, TimedDatum<S, T>> {
	T mAdd;

	using FAWIB = FilterActionWithInputBase<IOWrapper, TimedDatum<S, T>, CircBufL, std::allocator, TimedDatum<S, T>>;
	using FAWIB::getInputData;
public:
	explicit StatAddFilterAction(T&& add) : 
		FAWIB(std::shared_ptr<PostboxBase<TimedDatum<S, T>, CircBufL>>(new SimplePostbox< TimedDatum<S, T>>())),
		mAdd(std::move(add)) {};

	TimedDatum<S, T> actOn() override {

		CircBufL<TimedDatum<S, T>> vec;
		getInputData<CircBufL<TimedDatum<S, T>>, 0>(vec);

		TimedDatum<S, T> ret;

		if (!vec.empty()) {
			ret.datum = vec.crbegin()->datum + mAdd;
			ret.t = vec.crbegin()->t;
		}
		return ret;
	};
};

#endif

