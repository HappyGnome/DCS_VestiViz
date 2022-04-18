#pragma once

#ifndef _SIMPLEPOSTBOX_H_
#define _SIMPLEPOSTBOX_H_

#include "CircPostbox.h"

template <typename T,typename L>
class SimplePostbox : public CircPostbox<T,L> {
public:
	SimplePostbox() :CircPostbox<T, L>(1) {};
};


#endif