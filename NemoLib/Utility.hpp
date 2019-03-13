#pragma once

#ifndef __UTILITY__HPP__
#define __UTILITY__HPP__

#include <random>
#include <numeric>
#include <execution>


#ifndef _min
#define _min(a,b) (a > b ? b : a)
#endif

#ifndef _max
#define _max(a,b) (a > b ? a : b)
#endif

inline std::mt19937& RNG_provider(void)
{
	static std::random_device rd{};
	static std::mt19937 rng{rd()};

	return rng;
}


template<typename T>
T get_random_in_range(T min, T max)
{
	typedef std::conditional<std::is_integral<T>::value, std::uniform_int_distribution<T>, std::uniform_real_distribution<T>>::type dist_t;
	dist_t dist{ min, max };

	return dist(RNG_provider());
} // end template get_random_in_range


template <typename Iter, typename T>
inline T get_vector_sum(Iter begin, Iter end, T initial = 0)
{
	return std::reduce(std::execution::par_unseq, begin, end, initial, [&](auto cur, auto prev) { return cur + prev; });
}


#endif 
