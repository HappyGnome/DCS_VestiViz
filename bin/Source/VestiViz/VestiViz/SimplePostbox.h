#pragma once

#ifndef _SIMPLEPOSTBOX_H_
#define _SIMPLEPOSTBOX_H_

#include "CircPostbox.h"

template <typename T>
class SimplePostbox : public CircPostbox<T> {
public:
	SimplePostbox() :CircPostbox<T>(1) {};
};


#endif