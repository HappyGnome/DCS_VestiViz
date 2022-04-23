#pragma once

#include<list>

#include "SingleInputFilterBase.h"
#include "OutputFilterBase.h"
#include "CircPostbox.h"
#include "AccelByRegressionFilterAction.h"
#include"TimedDatum.h"

#ifndef _TEST_REGDIFFFILTER_H_
#define _TEST_REGDIFFFILTER_H_

using td = TimedDatum<float, float>;

struct RegDiffSIF : public SingleInputFilterBase<td, td, CircBufL> {
	explicit RegDiffSIF(std::size_t window) :SingleInputFilterBase<td, td, CircBufL>
		(std::make_shared<CircPostbox<td>>(window),
			std::make_unique<AccelByRegressionFilterAction<float, float, CircBufL>>()) {};
	virtual ~RegDiffSIF() = default;
};

#endif