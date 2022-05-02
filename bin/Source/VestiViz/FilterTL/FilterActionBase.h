#pragma once


#ifndef _FILTERACTIONBASE_H_
#define _FILTERACTIONBASE_H_

template <typename Tin, typename Tout, template<typename, typename> typename L, typename LAlloc = std::allocator<Tin>>
class FilterActionBase {
public:
	virtual ~FilterActionBase() = default;
	virtual Tout actOn(const L<Tin,LAlloc>& data) = 0;
};

template <typename Tin1,
	typename Tin2,
	typename Tout,
	template<typename, typename> typename L1,
	template<typename, typename> typename L2,
	typename LAlloc1 = std::allocator<Tin1>,
	typename LAlloc2 = std::allocator<Tin2> >
class DoubleFilterActionBase {
public:
	virtual ~DoubleFilterActionBase() = default;
	virtual Tout actOn(const L1<Tin1,LAlloc1> &data1, const L2<Tin2, LAlloc2>& data2) = 0;
};

#endif