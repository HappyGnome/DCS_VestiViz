#pragma once

#ifndef _TEST_MULTIPLYFILTER_H_
#define _TEST_MULTIPLYFILTER_H_

#include<list>

#include "SingleInputFilterBase.h"
#include "OutputFilterBase.h"
//#include "CircPostbox.h"
#include "SimplePostbox.h"
#include"TimedDatum.h"
#include "TestIOWrapper.h"

using TDf = TimedDatum<double, double> ;

class MultiplyAction:public FilterActionBase<TDf,TDf, CircBufL> {
	double mScale;
public:
	explicit MultiplyAction(double scale):mScale(scale){};
	virtual ~MultiplyAction() = default;

	TDf actOn(const CircBufL<TDf>& data) override {
		TDf res = TDf();
		res.datum = data.cbegin()->datum * mScale;
		res.t = data.cbegin()->t;
		return res;
	}
};

struct MultiplyProcessor : public SingleInputFilterBase<TDf, TDf, Test_IOWrapper, CircBufL> {
	explicit MultiplyProcessor(double scale) :SingleInputFilterBase<TDf, TDf, Test_IOWrapper, CircBufL>
		(std::make_shared<SimplePostbox<TDf>>(),
		 std::make_unique<MultiplyAction>(scale)) {};
	virtual ~MultiplyProcessor() {};
};


#endif