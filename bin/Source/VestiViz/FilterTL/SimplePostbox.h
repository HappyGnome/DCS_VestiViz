#pragma once

#ifndef _SIMPLEPOSTBOX_H_
#define _SIMPLEPOSTBOX_H_

#include "CircPostbox.h"

template <typename T, typename LAlloc = std::allocator<T>>
class SimplePostbox : public CircPostbox<T,LAlloc> {
public:
	SimplePostbox() :CircPostbox<T, LAlloc>(1) {};
};
#endif