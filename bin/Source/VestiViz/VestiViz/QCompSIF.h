#pragma once

#ifndef _QCOMPSIF_H_
#define _QCOMPSIF_H_


#include<list>

#include "SingleInputFilterBase.h"
#include "OutputFilterBase.h"
#include "QuickCompressFilterAction.h"
#include "SimplePostbox.h"
#include "ConvolveFilterAction.h"

template <typename S, typename T,
	typename LAlloc = std::allocator<TimedDatum<S, T>>>
struct QCompSIF : public SingleInputFilterBase<TimedDatum<S, T>, TimedDatum<S, T>, CircBufL, LAlloc> {
	explicit QCompSIF(T scale) :SingleInputFilterBase<TimedDatum<S, T>, TimedDatum<S, T>, CircBufL, LAlloc>
		(std::make_shared<SimplePostbox<TimedDatum<S, T>>>(),
			std::make_unique<QuickCompressFilterAction<S, T, CircBufL, LAlloc>>(scale)) {};
};

#endif