#pragma once

#ifndef _TEST_LOGGERFILTER_H_
#define _TEST_LOGGERFILTER_H_

#include<list>

#include "SingleInputFilterBase.h"
#include "OutputFilterBase.h"
//#include "CircPostbox.h"
#include "SimplePostbox.h"

class LogAction :public FilterActionBase<TimedDatum<float, float>, TimedDatum<float, float>, std::list> {
	std::string mPrefix;
public:
	LogAction(const std::string& prefix) :mPrefix(prefix) {};

	TimedDatum<float, float>  actOn(const std::list<TimedDatum<float, float>>& data) override {
		std::cout << mPrefix << data.crbegin()->datum << " t: "<< data.crbegin()->t <<std::endl;
		return *data.crbegin();
	}
};


struct LogSIF : public SingleInputFilterBase<TimedDatum<float, float>, TimedDatum<float, float>, std::list> {
	explicit LogSIF(const std::string& prefix) :SingleInputFilterBase<TimedDatum<float, float>, TimedDatum<float, float>, std::list>
		(std::make_shared<SimplePostbox<TimedDatum<float, float>, std::list>>(),
		 std::make_unique<LogAction>(prefix)) {};
};
struct LogOF : public OutputFilterBase<TimedDatum<float, float>, TimedDatum<float, float>, std::list> {
	explicit LogOF(const std::string& prefix) :OutputFilterBase<TimedDatum<float, float>, TimedDatum<float, float>, std::list>
		(1,
		std::make_unique<LogAction>(prefix)) {};
};

#endif