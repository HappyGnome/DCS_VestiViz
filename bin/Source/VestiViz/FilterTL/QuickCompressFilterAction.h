#pragma once

#ifndef _QUICKCOMPRESSFILTERACTION_H_
#define _QUICKCOMPRESSFILTERACTION_H_

#include <vector>
#include <list>
#include <math.h>

#include"Datalin.h"
#include"Datacomp.h"
#include "TimedDatum.h"
#include "FilterActionBase.h"

template <typename S, typename T, template<typename, typename> typename L, typename LAlloc = std::allocator<TimedDatum<S, T>>>
class QuickCompressFilterAction : public FilterActionBase<TimedDatum<S, T>, TimedDatum<S, T>, L, LAlloc> {
	T mCalib;
public:
	explicit QuickCompressFilterAction(T calibration) : mCalib(calibration) {};

	TimedDatum<S, T> actOn(const L<TimedDatum<S, T>, LAlloc>& data) override {
		if (data.empty()) return Datalin<S, TimedDatum<S, T>>::zero();

		TimedDatum<S, T> ret;

		TimedDatum<S, T> last = *(data.crbegin());

		ret.datum = Datacomp<S, T>::qComp(last.datum, mCalib);
		ret.t = last.t;

		return ret;
	};
};

#endif // !_QUICKCOMPRESSFILTERACTION_H_

