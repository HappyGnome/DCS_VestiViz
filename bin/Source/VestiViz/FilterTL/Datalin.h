#pragma once
#ifndef _DATALIN_H_
#define _DATALIN_H_

template <typename S, typename T>
class Datalin {
public:

	static T zero() {
		return T::zero();
	}

	/********************************
	* Lin - Generic.
	*********************************/
	static T lin(const T& x, const S scaleX, const T& y, const S scaleY) {
		return T::lin(x, scaleX, y, scaleY);
	}

	static T lin(const T& x, const T& y, const S scaleY) {
		return T::lin(x, y, scaleY);
	}

	/********************************
	* LinEq - Generic.
	*********************************/
	static T linEq(T& x, const S scaleX, const T& y, const S scaleY) {
		return x.linEq(scaleX, y, scaleY);
	}

	static T linEq(T& x, const T& y, const S scaleY) {
		return x.linEq(y, scaleY);
	}
};

template <typename S>
class Datalin<S,S>{
public:

	static S zero() {
		return (S) 0;
	}

	/********************************
	* Lin
	*********************************/
	static S lin(S x, S scaleX, S y, S scaleY) {
		return x * scaleX + y * scaleY;
	}
	static S lin(S x, S y, S scaleY) {
		return x + y * scaleY;
	}

	/********************************
	* LinEq
	*********************************/
	static S& linEq(S& x, S scaleX, S y, S scaleY) {
		x = x * scaleX + y * scaleY;
		return x;
	}
	static S& linEq(S& x, S y, S scaleY) {
		x += y * scaleY;
		return x;
	}

};
#endif