#pragma once

#ifndef _TEST_LOGGERFILTER_H_
#define _TEST_LOGGERFILTER_H_

#include<list>

#include "SingleInputFilterBase.h"
#include "OutputFilterBase.h"
//#include "CircPostbox.h"
#include "SimplePostbox.h"

template <typename S, typename T>
class LogAction :public FilterActionBase<TimedDatum<S, T>, TimedDatum<S, T>, CircBufL> {
	std::string mPrefix;
public:
	LogAction(const std::string& prefix) :mPrefix(prefix) {};

	TimedDatum<S, T>  actOn(const CircBufL<TimedDatum<S, T>>& data) override {
		std::cout << mPrefix << data.crbegin()->datum << " t: "<< data.crbegin()->t <<std::endl;
		return *data.crbegin();
	}
};

template <typename S, typename T>
struct LogSIF : public SingleInputFilterBase<TimedDatum<S, T>, TimedDatum<S, T>, CircBufL> {
	explicit LogSIF(const std::string& prefix) :SingleInputFilterBase<TimedDatum<S, T>, TimedDatum<S, T>, CircBufL>
		(std::make_shared<SimplePostbox<TimedDatum<S, T>>>(),
		 std::make_unique<LogAction<S,T>>(prefix)) {};
};

template <typename S, typename T>
struct LogOF : public OutputFilterBase<TimedDatum<S, T>, TimedDatum<S, T>, CircBufL> {
	explicit LogOF(const std::string& prefix) :OutputFilterBase<TimedDatum<S, T>, TimedDatum<S, T>, CircBufL>
		(std::make_shared<SimplePostbox<TimedDatum<S, T>>>(),
		std::make_unique<LogAction<S, T>>(prefix)) {};
};

#endif