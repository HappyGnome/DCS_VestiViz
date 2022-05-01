#pragma once

#ifndef _SIMPLEDIFFSIF_H_
#define _SIMPLEDIFFSIF_H_

#include<list>

#include "SingleInputFilterBase.h"
#include "CircPostbox.h"
#include "SimpleDiffFilterAction.h"
#include"TimedDatum.h"



template <typename S, typename T,
	typename IOWrapper,
	typename LAlloc = std::allocator<TimedDatum<S, T>>>
	struct SimpleDiffSIF : public SingleInputFilterBase<TimedDatum<S, T>, TimedDatum<S, T>, IOWrapper, CircBufL, LAlloc> {
	explicit SimpleDiffSIF() :SingleInputFilterBase<TimedDatum<S, T>, TimedDatum<S, T>, IOWrapper, CircBufL, LAlloc>
		(std::make_shared<CircPostbox<TimedDatum<S, T>, LAlloc>>(2),
			std::make_unique<SimpleDiffFilterAction<S, T, CircBufL, LAlloc>>()) {};
	virtual ~SimpleDiffSIF() = default;
};

#endif