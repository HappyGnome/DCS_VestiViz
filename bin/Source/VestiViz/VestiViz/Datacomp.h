#pragma once
#ifndef _DATACOMP_H_
#define _DATACOMP_H_

template <typename S, typename T>
class Datacomp {
public:

	static T qComp(const T& x, const T& scale) {
		return T::qComp(x, scale);
	}

	static T qCompEq(T& x, const T& scale) {
		return x.qCompEq(scale);
	}
};

template <typename S>
class Datacomp<S, S> {
public:

	static S qComp(S x, S scale) {
		return x/(std::abs(x)+scale);
	}

	static S& qCompEq(S& x, S scale) {
		x = x / (std::abs(x) + scale);
		return x;
	}

};
#endif