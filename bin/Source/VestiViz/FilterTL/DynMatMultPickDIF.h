#pragma once

#ifndef _DYNMATMULTOFFSETDIF_H_
#define _DYNMATMULTOFFSETDIF_H_

#include<list>

#include "DoubleInputFilterBase.h"
//#include "CircBuf.h"
#include "SimplePostbox.h"
#include "DynMatMultPickFilterAction.h"
#include "DatumMatrix.h"
#include "DatumArr.h"

template <
typename S,
typename T,
typename IOWrapper,
std::size_t M,
std::size_t N,
std::size_t K,
std::size_t L,
typename LAlloc1 = std::allocator<TimedDatum<S, DatumArr<S, T, K>>>,
typename LAlloc2 = std::allocator<TimedDatum<S, DatumMatrix<S, M, N>>>>
struct DynMatMultPickDIF : public DoubleInputFilterBase<
	TimedDatum<S, DatumArr<S, T, K>>,
	TimedDatum<S, DatumMatrix<S, M, N>>,
	TimedDatum<S, DatumArr<S, T, L>>,
	IOWrapper,
	CircBufL,
	CircBufL,
	LAlloc1,
	LAlloc2> {

	explicit DynMatMultPickDIF(std::array<std::tuple<std::size_t, std::size_t>, L>&& picks)
		: DoubleInputFilterBase<
		TimedDatum<S, DatumArr<S, T, K>>,
		TimedDatum<S, DatumMatrix<S, M, N>>,
		TimedDatum<S, DatumArr<S, T, L>>,
		IOWrapper,
		CircBufL,
		CircBufL,
		LAlloc1,
		LAlloc2>
	(std::make_shared<SimplePostbox<TimedDatum<S, DatumArr<S, T, K>>>>(),
	std::make_shared<SimplePostbox<TimedDatum<S, DatumMatrix<S, M, N>>>>(),
	std::make_unique<DynMatMultPickFilterAction<
	S,
	T,
	M,
	N,
	K,
	L,
	CircBufL,
	CircBufL,
	LAlloc1,
	LAlloc2>>(std::move(picks))) {};
};

#endif