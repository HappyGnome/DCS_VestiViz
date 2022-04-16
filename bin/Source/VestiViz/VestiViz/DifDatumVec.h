#pragma once

#ifndef _DifDatumVec_H_
#define _DifDatumVec_H_

#include <array>

/*********************************************
* Datum for differentiation algorithms
**********************************************/
template <typename S, int N> //scalar type
class DifDatumVec {
	std::array<S,N> mVec;

public:

	S& operator[] (std::size_t n) { return mVec[n]; }
	const S& operator[] (std::size_t n) const { return mVec[n]; }

	//Algebraic operators

	DifDatumVec operator * (const S scalar) const {
		DifDatumVec<S, N> ret;

		for (int i = 0; i < N; i++) {
			ret.mVec[i] = scalar * mVec[i];
		}
		return ret;
	}

	DifDatumVec& operator *= (const S scalar) {
		for (int i = 0; i < N; i++) {
			mVec[i] *= scalar;
		}
		return *this;
	}

	DifDatumVec operator + (const DifDatumVec<S, N>& other) const {
		DifDatumVec<S, N> ret;

		for (int i = 0; i < N; i++) {
			ret.mVec[i] = other.mVec[i] + mVec[i];
		}
		return ret;
	}
	DifDatumVec& operator += (const DifDatumVec<S, N>& other) {
		for (int i = 0; i < N; i++) {
			mVec[i] += other.mVec[i];
		}
		return *this;
	}
};

/*********************************************
* Datum for differentiation algorithms
**********************************************/
template <typename S, int N>
struct DifDatumVecT {
	DifDatumVec<S,N> vec;
	S t;
};

#endif