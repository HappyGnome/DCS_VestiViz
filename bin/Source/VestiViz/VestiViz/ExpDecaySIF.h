#pragma once

#ifndef _EXPDECAYSIF_H_
#define _EXPDECAYSIF_H_

#include<list>

#include "SingleInputFilterBase.h"
#include "OutputFilterBase.h"
//#include "CircPostbox.h"
#include "SimplePostbox.h"
#include "ExpDecayFilterAction.h"
#include"TimedDatum.h"

template <typename S, typename T,
	typename LAlloc = std::allocator<TimedDatum<S, T>>>
struct ExpDecaySIF : public SingleInputFilterBase<TimedDatum<S, T>, TimedDatum<S, T>, CircBufL,LAlloc> {
	explicit ExpDecaySIF(S halflife) :SingleInputFilterBase<TimedDatum<S, T>, TimedDatum<S, T>, CircBufL, LAlloc>
		(std::make_shared<SimplePostbox<TimedDatum<S, T>>>(),
		std::make_unique<ExpDecayFilterAction<S, T, CircBufL, LAlloc>>(halflife)) {};
	virtual ~ExpDecaySIF() = default;
};

#endif