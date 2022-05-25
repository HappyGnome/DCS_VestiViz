#pragma once

#ifndef _STATMATMULTPICKFILTERACTION_H_
#define _STATMATMULTPICKFILTERACTION_H_

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
	std::size_t M,//matrix dim
	std::size_t N,
	std::size_t K,//input
	std::size_t L>//output
	class StatMatMultPickFilterAction : public FilterActionWithInputBase <
	IOWrapper,
	TimedDatum<S, DatumArr<S, V, L>>,
	CircBufL, std::allocator,
	TimedDatum<S, DatumArr<S, V, K>>> {

	std::array<std::tuple<std::size_t, std::size_t>, L> mPicks;
	DatumMatrix<S, M, N> mMat;

	using FAWIB = FilterActionWithInputBase <
		IOWrapper,
		TimedDatum<S, DatumArr<S, V, L>>,
		CircBufL, std::allocator,
		TimedDatum<S, DatumArr<S, V, K>>>;
	using FAWIB::getInputData;

	public:
		explicit DynMatMultPickFilterAction(std::array<std::tuple<std::size_t, std::size_t>, L>&& picks, DatumMatrix<S, M, N>&& mat) :
			FAWIB(std::shared_ptr<PostboxBase<TimedDatum<S, DatumArr<S, V, K>>, CircBufL>>(new SimplePostbox< TimedDatum<S, DatumArr<S, V, K>>>())),
			mMat(std::move(mat)),
			mPicks(picks) {};

		TimedDatum<S, DatumArr<S, V, L>> actOn() override {

			CircBufL<TimedDatum<S, DatumArr<S, V, K>>> vec;

			getInputData<CircBufL<TimedDatum<S, DatumArr<S, V, K>>, std::allocator<TimedDatum<S, DatumArr<S, V, K>>>>, 0>(vec);

			TimedDatum<S, DatumArr<S, V, L>> ret;

			if (!vec.empty()) {
				ret.datum = mMat.applyAndPick<V, K, L>(vec.crbegin()->datum, mPicks);
				ret.t = vec.crbegin()->t;
			}
			return ret;
		};
};

#endif
