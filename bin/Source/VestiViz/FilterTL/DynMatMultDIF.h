#pragma once

#ifndef _DYNMATMULTDIF_H_
#define _DYNMATMULTDIF_H_

#include<list>

#include "DoubleInputFilterBase.h"
//#include "CircBuf.h"
#include "SimplePostbox.h"
#include "DynMatMultFilterAction.h"
#include "DatumMatrix.h"
#include "DatumArr.h"

template <
	typename S,
	typename T,
	typename IOWrapper,
	std::size_t M,
	std::size_t N,
	typename LAlloc1 = std::allocator<TimedDatum<S, DatumArr<S, T, N>>>,
	typename LAlloc2 = std::allocator<TimedDatum<S, DatumMatrix<S, M, N>>> >

struct DynMatMultDIF : public DoubleInputFilterBase<
			TimedDatum<S, DatumArr<S,T,N>>,
			TimedDatum<S, DatumMatrix<S, M, N>>,
			TimedDatum<S, DatumArr<S,T, M>>,
			IOWrapper,
			CircBufL,
			CircBufL,
			LAlloc1,
			LAlloc2> {
	explicit DynMatMultDIF()
		: DoubleInputFilterBase<
		TimedDatum<S, DatumArr<S, T, N>>,
		TimedDatum<S, DatumMatrix<S, M, N>>,
		TimedDatum<S, DatumArr<S, T, M>>,
		IOWrapper,
		CircBufL,
		CircBufL,
		LAlloc1,
		LAlloc2>
		(std::make_shared<SimplePostbox<TimedDatum<S, DatumArr<S, T, N>>>>(),
		 std::make_shared<SimplePostbox<TimedDatum<S, DatumMatrix<S, M, N>>>>(),
		 std::make_unique<DynMatMultFilterAction<
			S, 
			DatumArr<S, T, M>, 
			DatumArr<S, T, N>, 
			DatumMatrix<S, M, N>, 
			CircBufL,
			CircBufL,
			LAlloc1,
			LAlloc2>>()) {};
};

#endif