#pragma once

#ifndef _DYNMATMULTPICKFILTERACTION_H_
#define _DYNMATMULTPICKFILTERACTION_H_

#include <vector>
#include <list>
#include <math.h>

#include"Datalin.h"
#include"DatumMatrix.h"
#include "TimedDatum.h"
#include "FilterActionBase.h"

template <
	typename S,
	typename V,
	std::size_t M,
	std::size_t N,
	std::size_t K,
	std::size_t L,
	template<typename, typename> typename L1,
	template<typename, typename> typename L2,
	typename LAlloc1 = std::allocator<TimedDatum<S, DatumArr<S, V, K>>>,
	typename LAlloc2 = std::allocator<TimedDatum<S, DatumMatrix<S, M, N>>>>
	class DynMatMultPickFilterAction : public DoubleFilterActionBase<
	TimedDatum<S, DatumArr<S, V, K>>,
	TimedDatum<S, DatumMatrix<S, M, N>>,
	TimedDatum<S, DatumArr<S, V, L>>,
	L1,
	L2,
	LAlloc1,
	LAlloc2> {

	std::array<std::tuple<std::size_t,std::size_t>, L> mPicks;
	public:
		explicit DynMatMultPickFilterAction(std::array<std::tuple<std::size_t, std::size_t>, L>&& picks) :mPicks(picks) {};

		TimedDatum<S, DatumArr<S,V,L>> actOn(
			const L1<TimedDatum<S, DatumArr<S, V, K>>,
			LAlloc1>& vec,
			const L2<TimedDatum<S, DatumMatrix<S, M,N>>,
			LAlloc2>& matrix) override {

			TimedDatum<S, DatumArr<S, V, L>> ret;
	
			if (!vec.empty() && !matrix.empty()) {
				ret.datum = matrix.crbegin()->datum.applyAndPick<V,K,L>(vec.crbegin()->datum,mPicks);
				ret.t = (S)0.5 * (matrix.crbegin()->t + vec.crbegin()->t);
			}
			return ret;
		};
};

#endif
