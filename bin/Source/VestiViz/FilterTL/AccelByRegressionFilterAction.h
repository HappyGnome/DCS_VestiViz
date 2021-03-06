#pragma once

#ifndef _ACCELBYREGRESSIONFILTERACTION_H_
#define _ACCELBYREGRESSIONFILTERACTION_H_

#include <vector>
#include <list>
#include <math.h>

#include"Datalin.h"
#include "TimedDatum.h"
#include "FilterActionBase.h"

template <typename S, 
		  typename T, 
	      template<typename, typename> typename L,
	      typename LAlloc = std::allocator<TimedDatum<S, T>>>
class AccelByRegressionFilterAction : public FilterActionBase<TimedDatum<S, T>, TimedDatum<S, T>, L, LAlloc> {

public:
	TimedDatum<S, T> actOn(const L<TimedDatum<S, T>, LAlloc>& data) override {
		std::size_t n = data.size();
		TimedDatum<S, T> ret = TimedDatum<S, T>::zero();
		std::vector<S> taus(n, 0);
		std::vector<S> tau2s(n, 0);

		if (n == 0) return ret;

		S t = 0, tsum2 = 0, tsum3 = 0, tsum4 = 0;

		for (auto it = data.cbegin(); it != data.cend(); it++) {
			t += it->t;
		}
		t /= n;

		auto tauIt = taus.begin();
		auto tau2It = tau2s.begin();
		for (auto it = data.cbegin(); it != data.cend(); it++) {
			S tau = it->t - t;
			S tau2 = tau * tau;
			S tau3 = tau2 * tau;
			S tau4 = tau3 * tau;

			tsum2 += tau2;
			tsum3 += tau3;
			tsum4 += tau4;

			*tauIt = tau;
			*tau2It = tau2;

			tauIt++;
			tau2It++;
		}

		S D = tsum2 * tsum4 - tsum2 * tsum2 * tsum2 / n - tsum3 * tsum3;
		if (D == 0) return ret;


		S c = -2 * tsum2 * tsum2 / (n * D);
		S b = -2 * tsum3 / D;
		S a = 2 * tsum2 / D;

		tauIt = taus.begin();
		tau2It = tau2s.begin();
		for (auto it = data.cbegin(); it != data.cend(); it++) {
			
			Datalin<S, T>::linEq(ret.datum, it->datum, (*tau2It) * a + (*tauIt) * b + c);

			tauIt++;
			tau2It++;
		}

		ret.t = t;
		return ret;
	};
};

#endif // !_ACCELBYREGRESSIONFILTERACTION_H_

