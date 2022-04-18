#pragma once


#ifndef _FILTERACTIONBASE_H_
#define _FILTERACTIONBASE_H_

template <typename T, typename L>
class FilterActionBase {
public:
	virtual T actOn(const L& data) = 0;
};

#endif