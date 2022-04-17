#pragma once

#include<list>

#include "SingleInputFilterBase.h"
#include "OutputFilterBase.h"
//#include "CircPostbox.h"
#include "SimplePostbox.h"

#ifndef _TEST_MULTIPLYPROCESSOR_H_
#define _TEST_MULTIPLYPROCESSOR_H_

struct MultiplyProcessor : public SingleInputFilterBase<float, float, std::list<float>> {
	float scale = 1;
	explicit MultiplyProcessor() :SingleInputFilterBase<float, float, std::list<float>>(std::make_shared<SimplePostbox<float>>()) {};
protected:
	float processStep(const std::list<float>& data) override {
		float res = *data.begin() * scale;
		std::cout << "In: " << *data.begin() << " Out:" << res << std::endl;
		return res;
	}
};

struct AverageOutputProcessor : public OutputFilterBase<float, float> {
	explicit AverageOutputProcessor() :OutputFilterBase<float, float>(2) {};
protected:

	float processStep(const std::list<float>& data) override {
		float v = 0;
		std::size_t n = data.size();
		if (n > 0)
		{
			for (auto it = data.cbegin(); it != data.cend(); it++) {
				v += *it;
			}
			v /= (float)n;
		}
		std::cout << "Average of last outputs: " << v << std::endl;
		return v;
	}
};

#endif