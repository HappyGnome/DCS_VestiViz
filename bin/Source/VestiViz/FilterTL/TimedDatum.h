#pragma once

#ifndef _TIMEDDATUM_H_
#define _TIMEDDATUM_H_

#include "Datalin.h"

/*********************************************
* Datum for differentiation algorithms
**********************************************/
template <typename S, typename T>
struct TimedDatum {

	static TimedDatum zero() {
		auto ret = TimedDatum<S, T>();
		ret.t = 0;
		ret.datum = Datalin<S, T>::zero();
		return ret;
	}

	TimedDatum() = default;
	TimedDatum(S time, T&& d) :datum(std::move(d)), t(time) {};

	T datum;
	S t = 0;
};

#endif