#pragma once

#include<list>

#include "SingleInputFilterBase.h"
//#include "CircPostbox.h"
#include "SimplePostbox.h"

#ifndef _TEST_MULTIPLYPROCESSOR_H_
#define _TEST_MULTIPLYPROCESSOR_H_

struct MultiplyProcessor : public SingleInputFilterBase<float, float, float> {
	float scale = 1;
	//explicit MultiplyProcessor() :SingleInputFilterBase<float, float, std::list<float>>(std::make_shared<CircPostbox<float>>(1)) {};
	explicit MultiplyProcessor() :SingleInputFilterBase<float, float, float>(std::make_shared<SimplePostbox<float>>()) {};
	virtual ~MultiplyProcessor() = default;
protected:
	/*float processStep(const std::list<float>& data) override{
		float res = *data.cbegin() * scale;
		std::cout << "In: "<< *data.cbegin() <<" Out:"<<res<<std::endl;
		return res;
	}*/

	float processStep(const float& data) override {
		float res = data * scale;
		std::cout << "In: " << data << " Out:" << res << std::endl;
		return res;
	}
};

#endif