#pragma once

#include<list>

#include "SingleInputFilterBase.h"
#include "OutputFilterBase.h"
//#include "CircPostbox.h"
#include "SimplePostbox.h"
#include "ExpDecayFilterAction.h"
#include"TimedDatum.h"

#ifndef _TEST_EXPDECAYFILTER_H_
#define _TEST_EXPDECAYFILTER_H_

struct ExpSIF : public SingleInputFilterBase<TimedDatum<float, float>, TimedDatum<float, float>, std::list<TimedDatum<float, float>>> {
	explicit ExpSIF(float halflife) :SingleInputFilterBase<TimedDatum<float, float>, TimedDatum<float, float>, std::list<TimedDatum<float, float>>>
		(std::make_shared<SimplePostbox<TimedDatum<float, float>, std::list<TimedDatum<float, float>>>>(),
		std::make_unique<ExpDecayFilterAction<float, float, std::list<TimedDatum<float, float>>>>(halflife)) {};
	virtual ~ExpSIF() = default;
};

#endif