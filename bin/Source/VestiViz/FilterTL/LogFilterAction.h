#pragma once

#ifndef _LOGFILTERACTION_H_
#define _LOGFILTERACTION_H_

#include <iostream>
#include<list>

#include "FilterActionWithInputBase.h"
#include "SimplePostbox.h"

template <typename IOWrapper, typename S, typename T>
class LogFilterAction :public FilterActionWithInputBase<IOWrapper, TimedDatum<S, T>, TimedDatum<S, T>, CircBufL, std::allocator<TimedDatum<S, T>>> {
	std::string mPrefix;

	using FAWIB = FilterActionWithInputBase<IOWrapper, TimedDatum<S, T>, TimedDatum<S, T>, CircBufL, std::allocator<TimedDatum<S, T>>>;
	using FAWIB::getInputData;
public:
	explicit LogFilterAction(const std::string& prefix) :
		FAWIB(std::shared_ptr<PostboxBase<TimedDatum<S, T>, CircBufL>>(new SimplePostbox< TimedDatum<S, T>>())),
		mPrefix(prefix) {};

	TimedDatum<S, T>  actOn() override {
		CircBufL<TimedDatum<S, T>, std::allocator<TimedDatum<S, T>>> data;
		getInputData<CircBufL<TimedDatum<S, T>, std::allocator<TimedDatum<S, T>>>, 0>(data);

		std::cout << mPrefix << data.crbegin()->datum << " t: " << data.crbegin()->t << std::endl;
		return *data.crbegin();
	}
};

#endif