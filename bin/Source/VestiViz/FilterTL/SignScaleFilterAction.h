#pragma once

#ifndef _SIGNSCALEFILTERACTION_H_
#define _SIGNSCALEFILTERACTION_H_

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
	class SignScaleFilterAction : public FilterActionWithInputBase<IOWrapper, TimedDatum<S, T>, CircBufL, std::allocator, TimedDatum<S, T>> {
	T mSignScale;

	using FAWIB = FilterActionWithInputBase<IOWrapper, TimedDatum<S, T>, CircBufL, std::allocator, TimedDatum<S, T>>;
	using FAWIB::getInputData;
	public:
		explicit SignScaleFilterAction(T&& signScale) :
			FAWIB(std::shared_ptr<PostboxBase<TimedDatum<S, T>, CircBufL>>(new SimplePostbox< TimedDatum<S, T>>())),
			mSignScale(std::move(signScale)) {};

		TimedDatum<S, T> actOn() override {

			CircBufL<TimedDatum<S, T>> vec;
			getInputData<CircBufL<TimedDatum<S, T>>, 0>(vec);

			if (vec.empty()) return Datalin<S, TimedDatum<S, T>>::zero();

			auto data = vec.cbegin();
			
			return TimedDatum<S, T>(data->t,Datalin<S, T>::signScale(data->datum, mSignScale));
		};
};

#endif