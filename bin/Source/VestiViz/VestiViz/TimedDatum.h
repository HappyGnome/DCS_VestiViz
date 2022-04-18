#pragma once

#ifndef _TIMEDDATUM_H_
#define _TIMEDDATUM_H_

/*********************************************
* Datum for differentiation algorithms
**********************************************/
template <typename S, typename T>
struct TimedDatum {
	T datum;
	S t = 0;
};

#endif