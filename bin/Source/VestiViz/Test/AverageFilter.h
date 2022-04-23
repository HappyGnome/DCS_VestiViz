#pragma once

#include<list>

#include "SingleInputFilterBase.h"
#include "OutputFilterBase.h"
//#include "CircPostbox.h"
#include "SimplePostbox.h"
#include "ConvolveFilterAction.h"

#ifndef _TEST_AVERAGEFILTER_H_
#define _TEST_AVERAGEFILTER_H_

struct AverageOutputProcessor : public OutputFilterBase<TimedDatum<float, float>, TimedDatum<float, float>, std::list> {
	explicit AverageOutputProcessor() :OutputFilterBase<TimedDatum<float, float>, TimedDatum<float, float>, std::list>
		(8,
			std::make_unique<ConvolveFilterAction<float,float, std::list>>(std::vector<float>(7,0.5))) {};
};

#endif