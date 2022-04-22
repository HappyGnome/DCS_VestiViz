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

struct RegDiffSIF : public SingleInputFilterBase<td, td, std::list<td>> {
	explicit RegDiffSIF(std::size_t window) :SingleInputFilterBase<td, td, std::list<td>>
		(std::make_shared<CircPostbox<td, std::list<td>>>(window),
			std::make_unique<AccelByRegressionFilterAction<float, float, std::list<td>>>()) {};
	virtual ~RegDiffSIF() = default;
};

#endif