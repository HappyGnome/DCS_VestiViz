#pragma once

#ifndef _DATUMARR_H_
#define _DATUMARR_H_

#include <array>
#include<ostream>

#include "Datalin.h"
#include "Datacomp.h"

/*********************************************
* Vector datum for differentiation algorithms
**********************************************/
template <typename S,typename V, std::size_t N> 
class DatumArr {
	std::array<V,N> mVec;

public:

	explicit DatumArr(const std::array<V, N>&& v) : mVec(v) {};

	template <typename... Args>
	explicit DatumArr(Args... args) : mVec({ args... }) {};


	V& operator[] (std::size_t n) { return mVec[n]; }
	const V& operator[] (std::size_t n) const { return mVec[n]; }

	friend std::ostream& operator << (std::ostream& os, const DatumArr<S, V, N>& data) {
		os << "(";
		for (std::size_t i = 0; i < N-1; i++) {
			os << data[i] << ",";
		}
		if (N > 0) {
			os << data[N - 1];
		}
		os << ")";
		return os;
	}

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
			Datalin<S,V>::linEq(mVec[i], y.mVec[i], scaleY);
		}
		return *this;
	}

	DatumArr& linEq(const S scaleX, const DatumArr<S, V, N>& y, const S scaleY) {
		for (int i = 0; i < N; i++) {
			Datalin<S, V>::linEq(mVec[i], scaleX, y.mVec[i], scaleY);
		}
		return *this;
	}


	static DatumArr lin(const DatumArr<S, V, N>& x, const S scaleX, const DatumArr<S, V, N>& y, const S scaleY) {
		DatumArr<S, V, N> ret;

		for (int i = 0; i < N; i++) {
			ret.mVec[i] = Datalin<S, V>::lin(x.mVec[i], scaleX, y.mVec[i], scaleY);
		}
		return ret;
	}

	static DatumArr lin(const DatumArr<S, V, N>& x, const DatumArr<S, V, N>& y, const S scaleY) {
		DatumArr<S, V, N> ret;

		for (int i = 0; i < N; i++) {
			ret.mVec[i] = Datalin<S, V>::lin(x.mVec[i], y.mVec[i], scaleY);
		}
		return ret;
	}

	static DatumArr qComp(const DatumArr<S, V, N>& x, const DatumArr<S, V, N>& scale) {
		DatumArr<S, V, N> ret;

		for (int i = 0; i < N; i++) {
			ret.mVec[i] = Datacomp<S, V>::qComp(x.mVec[i], scale.mVec[i]);
		}
		return ret;
	}

	DatumArr& qCompEq(const DatumArr<S, V, N>& scale) {
		for (int i = 0; i < N; i++) {
			Datacomp<S, V>::qCompEq(mVec[i],scale.mVec[i]);
		}
		return *this;
	}
};

template<typename S>
using Vec3Datum = DatumArr<S, S, 3>; 
#endif