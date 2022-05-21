#pragma once

#ifndef _QUICKCOMPRESSFILTERACTION_H_
#define _QUICKCOMPRESSFILTERACTION_H_

#include <vector>
#include <list>
#include <math.h>

#include"Datalin.h"
#include"Datacomp.h"
#include "TimedDatum.h"
#include "SimplePostbox.h"
#include "FilterActionWithInputBase.h"

template <typename IOWrapper, typename S, typename T>
class QuickCompressFilterAction : public FilterActionWithInputBase<IOWrapper, TimedDatum<S, T>, CircBufL, std::allocator, TimedDatum<S, T>> {
	T mCalib;

	using FAWIB = FilterActionWithInputBase<IOWrapper, TimedDatum<S, T>, CircBufL, std::allocator, TimedDatum<S, T>>;
	using FAWIB::getInputData;
public:
	explicit QuickCompressFilterAction(T calibration) : 
		FAWIB(std::shared_ptr<PostboxBase<TimedDatum<S, T>, CircBufL>>(new SimplePostbox< TimedDatum<S, T>>())),
		mCalib(calibration) {};

	TimedDatum<S, T> actOn() override {

		CircBufL<TimedDatum<S, T>> data;
		getInputData<CircBufL<TimedDatum<S, T>>, 0>(data);

		if (data.empty()) return Datalin<S, TimedDatum<S, T>>::zero();

		TimedDatum<S, T> ret;

		TimedDatum<S, T> last = *(data.crbegin());

		ret.datum = Datacomp<S, T>::qComp(last.datum, mCalib);
		ret.t = last.t;

		return ret;
	};
};

#endif // !_QUICKCOMPRESSFILTERACTION_H_

