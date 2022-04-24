#pragma once

#ifndef _LINCOMBDIF_H_
#define _LINCOMBDIF_H_

#include<list>

#include "DoubleInputFilterBase.h"
//#include "CircBuf.h"
#include "SimplePostbox.h"
#include "LinCombFilterAction.h"


template <typename S, typename T,
	typename LAlloc = std::allocator<TimedDatum<S, T>>>

struct LinCombDIF : public DoubleInputFilterBase<
			TimedDatum<S, T>, 
			TimedDatum<S, T>, 
			TimedDatum<S, T>, 
			CircBufL,
			CircBufL, 
			LAlloc, 
			LAlloc> {
	explicit LinCombDIF(S scaleX, S scaleY) 
				: DoubleInputFilterBase<
				TimedDatum<S, T>, 
				TimedDatum<S, T>, 
				TimedDatum<S, T>, 
				CircBufL, 
				CircBufL, 
				LAlloc, 
				LAlloc>
		(std::make_shared<SimplePostbox<TimedDatum<S, T>>>(),
		 std::make_unique<LinCombFilterAction<S, T, CircBufL, LAlloc>>(scaleX,scaleY)) {};
};

#endif