#pragma once

#ifndef _STATMATMULTDIF_H_
#define _STATMATMULTDIF_H_

#include<list>

#include "SingleInputFilterBase.h"
//#include "CircBuf.h"
#include "SimplePostbox.h"
#include "StatMatMultFilterAction.h"
#include "DatumMatrix.h"
#include "DatumArr.h"

template <
	typename S, 
	typename T,
	typename IOWrapper,
	std::size_t M,
	std::size_t N,
	typename LAlloc = std::allocator<TimedDatum<S, DatumArr<S, T, N>>>>

struct StatMatMultSIF : public SingleInputFilterBase<
		TimedDatum<S, DatumArr<S, T, N>>,
		TimedDatum<S, DatumArr<S, T, M>>,
		IOWrapper,
		CircBufL,
		LAlloc> {
	explicit StatMatMultSIF(DatumMatrix<S, M, N>&& mat)
		: SingleInputFilterBase<
			TimedDatum<S, DatumArr<S, T, N>>,
			TimedDatum<S, DatumArr<S, T, M>>,
			IOWrapper,
			CircBufL,
			LAlloc>
		(std::make_shared<SimplePostbox<TimedDatum<S, DatumArr<S, T, N>>>>(),
		 std::make_unique<StatMatMultFilterAction<
			S,
			DatumArr<S, T, N>,
			DatumMatrix<S, M, N>,
			DatumArr<S, T, M>,
			CircBufL,
			LAlloc>>(std::move(mat))) {};
};

#endif