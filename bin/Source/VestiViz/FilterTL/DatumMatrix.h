#pragma once

#ifndef _DATUMMATRIX_H_
#define _DATUMMATRIX_H_

#include <array>
#include<ostream>

#include "DatumArr.h"

template <typename S, std::size_t M, std::size_t N>
class DatumMatrix : public DatumArr<S,S,M * N> {
public:
	template <typename... Args>
	explicit DatumMatrix(Args... args) : DatumArr<S, S, M * N>(args... ) {};

	friend std::ostream& operator << (std::ostream& os, const DatumMatrix<S, M, N>& data) {
		os <<std::endl;
		for (std::size_t i = 0; i < M; i++) {
			os<< "(";
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
};
#endif
