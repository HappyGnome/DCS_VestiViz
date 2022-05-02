#pragma once

#ifndef _LOGFILTERACTION_H_
#define _LOGFILTERACTION_H_

#include <iostream>
#include<list>

#include "SingleInputFilterBase.h"
#include "OutputFilterBase.h"
//#include "CircPostbox.h"
#include "SimplePostbox.h"

template <typename S, typename T>
class LogFilterAction :public FilterActionBase<TimedDatum<S, T>, TimedDatum<S, T>, CircBufL> {
	std::string mPrefix;
public:
	LogFilterAction(const std::string& prefix) :mPrefix(prefix) {};

	TimedDatum<S, T>  actOn(const CircBufL<TimedDatum<S, T>>& data) override {
		std::cout << mPrefix << data.crbegin()->datum << " t: " << data.crbegin()->t << std::endl;
		return *data.crbegin();
	}
};

#endif