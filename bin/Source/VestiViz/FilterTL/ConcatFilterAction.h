#pragma once

#ifndef _CONCATFILTERACTION_H_
#define _CONCATFILTERACTION_H_

#include <vector>
#include <list>
#include <math.h>

#include"Datalin.h"
#include "SimplePostbox.h"
#include "TimedDatum.h"
#include "DatumArr.h"
#include "FilterActionWithInputBase.h"

template <
	typename IOWrapper,
	typename S,
	typename V,
	std::size_t M,
	std::size_t N>
	class ConcatFilterAction : public FilterActionWithInputBase<
	IOWrapper,
	TimedDatum<S, DatumArr<S,V,N+M>>,
	CircBufL, std::allocator,
	TimedDatum<S, DatumArr<S, V, M>>,
	TimedDatum<S, DatumArr<S, V, N>>> {

	using FAWIB = FilterActionWithInputBase<
		IOWrapper,
		TimedDatum<S, DatumArr<S, V, N + M>>,
		CircBufL, std::allocator,
		TimedDatum<S, DatumArr<S, V, M>>,
		TimedDatum<S, DatumArr<S, V, N>>>;

	using FAWIB::getInputData;
	public:
		explicit ConcatFilterAction() :
			FAWIB(std::shared_ptr<PostboxBase<TimedDatum<S, DatumArr<S, V, M>>, CircBufL>>(new SimplePostbox< TimedDatum<S, DatumArr<S, V, M>>>()),
				std::shared_ptr<PostboxBase<TimedDatum<S, DatumArr<S, V, N>>, CircBufL>>(new SimplePostbox< TimedDatum<S, DatumArr<S, V, N>>>()))
		{};

		TimedDatum<S, DatumArr<S, V, M + N>> actOn() override {

			CircBufL<TimedDatum<S, DatumArr<S, V, M>>> vec1;
			CircBufL<TimedDatum<S, DatumArr<S, V, N>>> vec2;

			getInputData<CircBufL<TimedDatum<S, DatumArr<S, V, M>>>, 0>(vec1);
			getInputData<CircBufL<TimedDatum<S, DatumArr<S, V, N>>>, 1>(vec2);

			auto pt2 = vec2.crbegin();
			auto pt1 = vec1.crbegin();
			TimedDatum<S, DatumArr<S, V, M + N>> ret;
			ret.datum = pt1->datum.Concat<4>(pt2->datum);
			ret.t = (pt1->t + pt2->t) * 0.5;
			return ret;
		};
};

#endif
