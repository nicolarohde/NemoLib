#pragma once

#ifndef __UTIL_HPP
#define __UTIL_HPP

#include "Config.hpp"
#include <random>
#include <numeric>
#include <chrono>
#include <cmath>

#ifndef _min
#define _min(a,b) (a > b ? b : a)
#endif

#ifndef _max
#define _max(a,b) (a > b ? a : b)
#endif


// Typedefs to make the clock and timepoint names shorter
typedef std::chrono::high_resolution_clock	_Clock;

inline std::mt19937& RNG_provider(void)
{
	static std::random_device rd{};
	static std::mt19937 rng{rd()};

	return rng;
} // end method RNG_provider


template<typename T>
T get_random_in_range(T min, T max)
{
	typedef typename std::conditional<std::is_integral<T>::value, std::uniform_int_distribution<T>, std::uniform_real_distribution<T>>::type dist_t;
	dist_t dist{ min, max };

	return dist(RNG_provider());
} // end template get_random_in_range


template <typename Iter, typename T = int>
inline T get_vector_sum(Iter begin, Iter end, T initial = 0)
{
	#if _C17_EXECUTION_AVAILABLE
		return std::reduce(std::execution::par_unseq, begin, end, initial);
	#else
		return std::accumulate(begin, end, initial);
	#endif
} // end template get_vector_sum


template <typename Iter, typename T, typename F>
inline T get_vector_sum(Iter begin, Iter end, T initial = 0, F lambda = std::plus<T>())
{
	#if _C17_EXECUTION_AVAILABLE
		return std::reduce(std::execution::par_unseq, begin, end, initial, lambda);
	#else
		return std::accumulate(begin, end, initial, lambda);
	#endif
} // end template get_vector_sum


template <typename D, typename T>
inline double chrono_duration(T start, T end)
{
	return static_cast<double>(std::chrono::duration_cast<D>(end - start).count());
} // end template chrono_duration


#endif
