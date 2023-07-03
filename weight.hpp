#ifndef LPNN_WEIGHT_HPP_
#define LPNN_WEIGHT_HPP_

namespace LPNN {
	template<typename T>
	class Weight {
		constexpr Weight(const T current) : m_current(current), m_delta(0) {}
		constexpr Weight(const T current, const T delta) : m_current(current), m_delta(delta) {}
		T m_current;
		T m_delta;
		Weight& updateWeight(const T delta) {
			m_current += (m_delta = delta);
			return *this;
		}
	};
}

#endif