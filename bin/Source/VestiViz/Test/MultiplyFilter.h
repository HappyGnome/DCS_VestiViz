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

class MultiplyAction:public FilterActionBase<TDf,TDf, CircBufL> {
	float mScale;
public:
	explicit MultiplyAction(float scale):mScale(scale){};
	virtual ~MultiplyAction() = default;

	TDf actOn(const CircBufL<TDf>& data) override {
		TDf res = TDf();
		res.datum = data.cbegin()->datum * mScale;
		res.t = data.cbegin()->t;
		return res;
	}
};

struct MultiplyProcessor : public SingleInputFilterBase<TDf, TDf, CircBufL> {
	explicit MultiplyProcessor(float scale) :SingleInputFilterBase<TDf, TDf, CircBufL>
		(std::make_shared<SimplePostbox<TDf>>(),
		 std::make_unique<MultiplyAction>(scale)) {};
	virtual ~MultiplyProcessor() {};
};


#endif