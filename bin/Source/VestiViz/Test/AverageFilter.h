#pragma once

#include<list>

#include "SingleInputFilterBase.h"
#include "OutputFilterBase.h"
//#include "CircPostbox.h"
#include "SimplePostbox.h"

#ifndef _TEST_AVERAGEFILTER_H_
#define _TEST_AVERAGEFILTER_H_


class AverageAction :public FilterActionBase<TimedDatum<float,float>, std::list<TimedDatum<float, float>>> {
public:
	TimedDatum<float, float>  actOn(const std::list<TimedDatum<float, float>>& data) override {
		TimedDatum<float, float> v;
		v.datum = 0;
		v.t = 0;

		std::size_t n = data.size();
		if (n > 0)
		{
			for (auto it = data.cbegin(); it != data.cend(); it++) {
				v.datum += it->datum;
				v.t += it->t;
			}
			v.datum /= (float)n;
			v.t /= (float)n;
		}
		return v;
	}
};



struct AverageOutputProcessor : public OutputFilterBase<TimedDatum<float, float>, TimedDatum<float, float>, std::list<TimedDatum<float, float>>> {
	explicit AverageOutputProcessor() :OutputFilterBase<TimedDatum<float, float>, TimedDatum<float, float>, std::list<TimedDatum<float, float>>>
		(2,
			std::make_unique<AverageAction>()) {};
};

#endif