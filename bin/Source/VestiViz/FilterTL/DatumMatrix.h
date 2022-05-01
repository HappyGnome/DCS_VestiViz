#pragma once

#ifndef _DATUMMATRIX_H_
#define _DATUMMATRIX_H_

#include <array>
#include<ostream>

#include "DatumArr.h"

template <typename S, std::size_t M, std::size_t N>
class DatumMatrix : public DatumArr<S, S, M* N> {
public:
	template <typename... Args>
	explicit DatumMatrix(Args... args) : DatumArr<S, S, M* N>(args...) {};

	friend std::ostream& operator << (std::ostream& os, const DatumMatrix<S, M, N>& data) {
		os << std::endl;
		for (std::size_t i = 0; i < M; i++) {
			os << "(";
			for (std::size_t j = 0; j < N - 1; j++) {
				os << data[N * i + j] << ",";
			}
			if (N > 0) {
				os << data[N * i + N - 1];
			}
			os << ")" << std::endl;
		}

		return os;
	}

	template<typename V>
	DatumArr<S, V, M> applyTo(const DatumArr<S, V, N>& vec) const{
		DatumArr<S, V, M> ret = DatumArr<S, V, M>::zero();

		for (std::size_t i = 0; i < M; i++) {
			std::size_t off = i * N;
			for (std::size_t j = 0; j < N; j++) {
				Datalin<S, V>::linEq(ret[i], vec[j], (*this)[off + j]);
			}
		}
		return ret;
	}

	template<typename V, std::size_t K, std::size_t L>
	DatumArr<S, V, L> applyAndPick(const DatumArr<S, V, K>& vec, const std::array<std::tuple<std::size_t, std::size_t>, L>& mPicks) const {
		DatumArr<S, V, L> ret = DatumArr<S, V, L>::zero();

		for (std::size_t i = 0; i < L; i++) {
			std::size_t offMat = std::get<0>(mPicks[i]) * N;
			std::size_t offVec = std::get<1>(mPicks[i]) * N;
			for (std::size_t j = 0; j < N; j++) {
				Datalin<S, V>::linEq(ret[i], vec[j + offVec], (*this)[offMat + j]);
			}
		}
		return ret;
	}
};
#endif
