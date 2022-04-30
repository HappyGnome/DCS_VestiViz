#pragma once

#include<list>

#include "SingleInputFilterBase.h"
#include "OutputFilterBase.h"
//#include "CircBuf.h"
#include "SimplePostbox.h"
#include "ConvolveFilterAction.h"

#ifndef _CONVOUTF_H_
#define _CONVOUTF_H_

template <typename S, typename T,
	typename IOWrapper,
	typename LAlloc = std::allocator<TimedDatum<S, T>>>
struct ConvOutF : public OutputFilterBase<TimedDatum<S, T>, TimedDatum<S, T>, IOWrapper, CircBufL, LAlloc> {
	explicit ConvOutF(std::vector<S>&& kernel) : OutputFilterBase<TimedDatum<S, T>, TimedDatum<S, T>, IOWrapper, CircBufL, LAlloc>
		(std::make_shared<CircPostbox<TimedDatum<S, T>, LAlloc>>(kernel.size()),
			std::make_unique<ConvolveFilterAction<S, T, CircBufL, LAlloc>>(std::move(kernel))) {};
};

#endif