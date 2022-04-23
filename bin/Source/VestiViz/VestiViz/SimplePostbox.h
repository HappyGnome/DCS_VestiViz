#pragma once

#ifndef _SIMPLEPOSTBOX_H_
#define _SIMPLEPOSTBOX_H_

#include "CircPostbox.h"

template <typename T,template<typename,typename> typename L, typename LAlloc = std::allocator<T>>
class SimplePostbox : public CircPostbox<T,L,LAlloc> {
public:
	SimplePostbox() :CircPostbox<T, L,LAlloc>(1) {};
};


#endif