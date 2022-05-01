#pragma once

#ifndef _LOGSIF_H_
#define _LOGSIF_H_

#include <iostream>
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
		std::cout << mPrefix << data.crbegin()->datum << " t: " << data.crbegin()->t << std::endl;
		return *data.crbegin();
	}
};

template <typename S, typename T, typename IOWrapper>
struct LogSIF : public SingleInputFilterBase<TimedDatum<S, T>, TimedDatum<S, T>, IOWrapper, CircBufL> {
	explicit LogSIF(const std::string& prefix) :SingleInputFilterBase<TimedDatum<S, T>, TimedDatum<S, T>, IOWrapper, CircBufL>
		(std::make_shared<SimplePostbox<TimedDatum<S, T>>>(),
			std::make_unique<LogAction<S, T>>(prefix)) {};
};

template <typename S, typename T, typename IOWrapper>
struct LogOF : public OutputFilterBase<TimedDatum<S, T>, TimedDatum<S, T>, IOWrapper, CircBufL> {
	explicit LogOF(const std::string& prefix) :OutputFilterBase<TimedDatum<S, T>, TimedDatum<S, T>, IOWrapper, CircBufL>
		(std::make_shared<SimplePostbox<TimedDatum<S, T>>>(),
			std::make_unique<LogAction<S, T>>(prefix)) {};
};

#endif