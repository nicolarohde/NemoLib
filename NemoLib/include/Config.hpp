#pragma once

#ifndef __NEMO_CONFIG
	#define __NEMO_CONFIG

	#ifndef _USE_THREAD_POOL
		#define _USE_THREAD_POOL 1	// use ThreadPool class for parallelization
	#endif

	#ifndef _USE_CUDA
		#define _USE_CUDA 0			// use CUDA code for random graph building
	#endif

	#ifdef __has_include
		#define _CAN_CHECK_INCLUDE
	#endif

	#ifdef _CAN_CHECK_INCLUDE
		#if __has_include(<execution>)
			#define _C17_EXECUTION_AVAILABLE 1
			#include <execution>
		#else
			#define _C17_EXECUTION_AVAILABLE 0
		#endif
	#else
		#define _C17_EXECUTION_AVAILABLE 0
	#endif

    #ifndef DEBUG_LEVEL
        #if NDEBUG
            #define DEBUG_LEVEL 4
        #else
            #define DEBUG_LEVEL 0
        #endif
    #endif

#endif // end __NEMO_CONFIG
