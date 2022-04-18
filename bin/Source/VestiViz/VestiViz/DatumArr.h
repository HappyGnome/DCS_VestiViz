#pragma once

#ifndef _DATUMARR_H_
#define _DATUMARR_H_

#include <array>

#include "datalin.h"

template <typename S, typename V, std::size_t N> //scalar type
class DatumArr;

/*********************************************
* Vector datum for differentiation algorithms
**********************************************/
template <typename S,typename V, std::size_t N> //scalar type
class DatumArr {
	std::array<V,N> mVec;

public:

	V& operator[] (std::size_t n) { return mVec[n]; }
	const V& operator[] (std::size_t n) const { return mVec[n]; }

	//Algebraic operators

	DatumArr operator * (const S scalar) const {
		DatumArr<S, V, N> ret;

		for (int i = 0; i < N; i++) {
			ret.mVec[i] = mVec[i] * scalar;
		}
		return ret;
	}

	DatumArr& operator *= (const S scalar) {
		for (int i = 0; i < N; i++) {
			mVec[i] *= scalar;
		}
		return *this;
	}

	DatumArr operator + (const DatumArr<S,V, N>& other) const {
		DatumArr<S, V, N> ret;

		for (int i = 0; i < N; i++) {
			ret.mVec[i] = other.mVec[i] + mVec[i];
		}
		return ret;
	}
	DatumArr& operator += (const DatumArr<S,V, N>& other) {
		for (int i = 0; i < N; i++) {
			mVec[i] += other.mVec[i];
		}
		return *this;
	}

	DatumArr operator - (const DatumArr<S, V, N>& other) const {
		DatumArr<S, V, N> ret;

		for (int i = 0; i < N; i++) {
			ret.mVec[i] = other.mVec[i] - mVec[i];
		}
		return ret;
	}

	DatumArr& operator -= (const DatumArr<S, V, N>& other) {
		for (int i = 0; i < N; i++) {
			mVec[i] -= other.mVec[i];
		}
		return *this;
	}

	DatumArr& linEq(const DatumArr<S, V, N>& y, const S scaleY) {
		for (int i = 0; i < N; i++) {
			mVec[i] = datum::lin(mVec[i], y.mVec[i], scaleY);
		}
		return *this;
	}

	DatumArr& linEq(const S scaleX, const DatumArr<S, V, N>& y, const S scaleY) {
		for (int i = 0; i < N; i++) {
			mVec[i] = datum::lin(mVec[i], scaleX, y.mVec[i], scaleY);
		}
		return *this;
	}


	static DatumArr lin(const DatumArr<S, V, N>& x, const S scaleX, const DatumArr<S, V, N>& y, const S scaleY) {
		DatumArr<S, V, N> ret;

		for (int i = 0; i < N; i++) {
			ret.mVec[i] = lin(x.mVec[i], scaleX, y.mVec[i], scaleY);
		}
		return ret;
	}

	static DatumArr<S, V, N> lin(const DatumArr<S, V, N>& x, const DatumArr<S, V, N>& y, const S scaleY) {
		DatumArr<S, V, N> ret;

		for (int i = 0; i < N; i++) {
			ret.mVec[i] = lin(x.mVec[i], y.mVec[i], scaleY);
		}
		return ret;
	}
};
#endif