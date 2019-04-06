#pragma once

#ifndef __NEMO_CONFIG
#define __NEMO_CONFIG

#define _USE_THREAD_POOL 1	// use ThreadPool class for parallelization
#define _USE_CUDA 0			// use CUDA code for random graph building

#if defined(__has_include)
	#define _CAN_CHECK_INCLUDE
#endif

#ifdef _CAN_CHECK_INCLUDE
	#if __has_include(<execution>)
		#define _C17_EXECUTION_AVAILABLE 1
	#else
		#define _C17_EXECUTION_AVAILABLE 0
	#endif
#else
	#define _C17_EXECUTION_AVAILABLE 0
#endif

#if _C17_EXECUTION_AVAILABLE
#include <execution>
template <typename Iter, typename T, typename F>
T reduce_values(Iter begin, Iter end, T initial, F lambda = std::plus<>{})
{
	return std::reduce(std::execution::par_unseq, begin, end, initial, lambda);
}
#else
template <typename Iter, typename T, typename F>
T reduce_values(Iter begin, Iter end, T initial, F lambda)
{
	return std::accumulate(begin, end, initial, lambda);
}
#endif

#endif // end __NEMO_CONFIG
