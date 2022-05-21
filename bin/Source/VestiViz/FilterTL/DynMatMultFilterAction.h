#pragma once

#ifndef _DYNMATMULTFILTERACTION_H_
#define _DYNMATMULTFILTERACTION_H_

#include <vector>
#include <list>
#include <math.h>

#include"Datalin.h"
#include "TimedDatum.h"
#include "SimplePostbox.h"
#include "FilterActionWithInputBase.h"

template <
	typename IOWrapper,
	typename S,
	typename Tout,
	typename Tin1,
	typename Tin2>
class DynMatMultFilterAction : public FilterActionWithInputBase <
			IOWrapper,
			TimedDatum<S, Tout>,
			CircBufL,
			std::allocator,
			TimedDatum<S, Tin1>,				
			TimedDatum<S, Tin2>> {

	using FAWIB = FilterActionWithInputBase <
		IOWrapper,
		TimedDatum<S, Tout>,
		CircBufL,
		std::allocator,
		TimedDatum<S, Tin1>,
		TimedDatum<S, Tin2>>;
	using FAWIB::getInputData;
public:
	explicit DynMatMultFilterAction() :
		FAWIB(std::shared_ptr<PostboxBase<TimedDatum<S, Tin1>, CircBufL>>(new SimplePostbox< TimedDatum<S, Tin1>>()),
			  std::shared_ptr<PostboxBase<TimedDatum<S, Tin2>, CircBufL>>(new SimplePostbox< TimedDatum<S, Tin2>>())) {}

	TimedDatum<S, Tout> actOn() override {
		
		CircBufL<TimedDatum<S, Tin1>> vec;
		CircBufL<TimedDatum<S, Tin2>> matrix;

		getInputData<CircBufL<TimedDatum<S, Tin1>>, 0>(vec);
		getInputData<CircBufL<TimedDatum<S, Tin2>>, 1>(matrix);

		TimedDatum<S, Tout> ret;

		if (!vec.empty() && !matrix.empty()) {
			ret.datum = matrix.crbegin()->datum.applyTo(vec.crbegin() -> datum);
			ret.t = (S)0.5 * (matrix.crbegin()->t + vec.crbegin()->t);
		}
		return ret;
	};
};

#endif
