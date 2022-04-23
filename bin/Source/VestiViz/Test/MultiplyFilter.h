#pragma once

#ifndef _TEST_MULTIPLYFILTER_H_
#define _TEST_MULTIPLYFILTER_H_

#include<list>

#include "SingleInputFilterBase.h"
#include "OutputFilterBase.h"
//#include "CircPostbox.h"
#include "SimplePostbox.h"
#include"TimedDatum.h"

using TDf = TimedDatum<float, float> ;

class MultiplyAction:public FilterActionBase<TDf,TDf, std::list> {
	float mScale;
public:
	explicit MultiplyAction(float scale):mScale(scale){};
	virtual ~MultiplyAction() = default;

	TDf actOn(const std::list<TDf>& data) override {
		TDf res = TDf();
		res.datum = data.begin()->datum * mScale;
		res.t = data.begin()->t;
		return res;
	}
};

struct MultiplyProcessor : public SingleInputFilterBase<TDf, TDf, std::list> {
	explicit MultiplyProcessor(float scale) :SingleInputFilterBase<TDf, TDf, std::list>
		(std::make_shared<SimplePostbox<TDf, std::list>>(),
		 std::make_unique<MultiplyAction>(scale)) {};
	virtual ~MultiplyProcessor() {};
};


#endif