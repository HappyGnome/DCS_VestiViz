#pragma once

#ifndef _DYNMATMULTPICKFILTERACTION_H_
#define _DYNMATMULTPICKFILTERACTION_H_

#include <vector>
#include <list>
#include <math.h>

#include"Datalin.h"
#include"DatumMatrix.h"
#include "CircPostbox.h"
#include "TimedDatum.h"
#include "FilterActionWithInputBase.h"

template <
	typename IOWrapper,
	typename S,
	typename V,
	std::size_t M,
	std::size_t N,
	std::size_t K,
	std::size_t L>
	class DynMatMultPickFilterAction : public FilterActionWithInputBase <
												IOWrapper,
												TimedDatum<S, DatumArr<S, V, L>>,
												TimedDatum<S, DatumArr<S, V, K>>,
												CircBufL, std::allocator<TimedDatum<S, TimedDatum<S, DatumArr<S, V, K>>>>,
												TimedDatum<S, DatumMatrix<S, M, N>>,
												CircBufL, std::allocator<TimedDatum<S, TimedDatum<S, DatumMatrix<S, M, N>>>>> {

	std::array<std::tuple<std::size_t,std::size_t>, L> mPicks;

	using FAWIB = FilterActionWithInputBase <
		IOWrapper,
		TimedDatum<S, DatumArr<S, V, L>>,
		TimedDatum<S, DatumArr<S, V, K>>,
		CircBufL, std::allocator<TimedDatum<S, TimedDatum<S, DatumArr<S, V, K>>>>,
		TimedDatum<S, DatumMatrix<S, M, N>>,
		CircBufL, std::allocator<TimedDatum<S, TimedDatum<S, DatumMatrix<S, M, N>>>>>;
	using FAWIB::getInputData;
	public:
		explicit DynMatMultPickFilterAction(std::array<std::tuple<std::size_t, std::size_t>, L>&& picks) :
			FAWIB(std::shared_ptr<PostboxBase<TimedDatum<S, Tin1>, CircBufL>>(new SimplePostbox< TimedDatum<S, DatumArr<S, V, K>>>()),
				std::shared_ptr<PostboxBase<TimedDatum<S, Tin2>, CircBufL>>(new SimplePostbox< TimedDatum<S, DatumMatrix<S, M, N>>>())),
			mPicks(picks) {};

		TimedDatum<S, DatumArr<S,V,L>> actOn() override {

			CircBufL<TimedDatum<S, DatumArr<S, V, K>>> vec;
			CircBufL<TimedDatum<S, DatumMatrix<S, M, N>>> matrix;

			getInputData<L1<TimedDatum<S, Tin1>, LAlloc1>, 0>(vec);
			getInputData<L2<TimedDatum<S, Tin2>, LAlloc2>, 1>(matrix);

			TimedDatum<S, DatumArr<S, V, L>> ret;
	
			if (!vec.empty() && !matrix.empty()) {
				ret.datum = matrix.crbegin()->datum.applyAndPick<V,K,L>(vec.crbegin()->datum,mPicks);
				ret.t = (S)0.5 * (matrix.crbegin()->t + vec.crbegin()->t);
			}
			return ret;
		};
};

#endif
