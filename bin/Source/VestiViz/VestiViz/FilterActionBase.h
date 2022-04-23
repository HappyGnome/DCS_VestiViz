#pragma once


#ifndef _FILTERACTIONBASE_H_
#define _FILTERACTIONBASE_H_

template <typename Tin, typename Tout, template<typename, typename> typename L, typename LAlloc = std::allocator<Tin>>
class FilterActionBase {
public:
	virtual Tout actOn(const L<Tin,LAlloc>& data) = 0;
};

template <typename T, typename... L>
class MultiFilterActionBase {
public:
	virtual T actOn(const L &...data) = 0;
};

#endif