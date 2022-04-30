#pragma once

#ifndef _REGDIFFSIF_H_
#define _REGDIFFSIF_H_

#include<list>

#include "SingleInputFilterBase.h"
#include "OutputFilterBase.h"
#include "CircPostbox.h"
#include "AccelByRegressionFilterAction.h"
#include"TimedDatum.h"



template <typename S, typename T,
	typename LAlloc = std::allocator<TimedDatum<S, T>>>
struct RegDiffSIF : public SingleInputFilterBase<TimedDatum<S, T>, TimedDatum<S, T>, PIB_Wrapper, CircBufL, LAlloc> {
	explicit RegDiffSIF(std::size_t window) :SingleInputFilterBase<TimedDatum<S, T>, TimedDatum<S, T>, PIB_Wrapper, CircBufL, LAlloc>
		(std::make_shared<CircPostbox<TimedDatum<S, T>, LAlloc>>(window),
			std::make_unique<AccelByRegressionFilterAction<S, T, CircBufL, LAlloc>>()) {};
	virtual ~RegDiffSIF() = default;
};

#endif