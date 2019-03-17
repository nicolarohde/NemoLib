#pragma once

#ifndef __SHUFFLE_KERNELS_CUH
#define __SHUFFLE_KERNELS_CUH

#include <cstddef>
#include <cuda.h>
#include <iostream>
#include <string.h>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "curand_kernel.h"

#ifndef CUDA_CALL
#define CUDA_CALL(x) do { auto y = (x); if(y != cudaSuccess) { \
    printf("Error at %s:%d\n",__FILE__,__LINE__); \
	std::cout << "Error is: " << cudaGetErrorString(y) << std::endl; \
    exit(EXIT_FAILURE);}} while(0)
#endif

typedef unsigned int uint;


__global__ void setup_kernel(curandState *state)
{
	int id = threadIdx.x + blockIdx.x * blockDim.x;
	/* Each thread gets same seed, a different sequence
	number, no offset */
	curand_init(clock64(), id, 0, &state[id]);
}


template <typename T>
__device__ void shuffle_vector(T* __restrict vect, const std::size_t LENGTH, const std::size_t idx_2, curandState *state)
{
	std::size_t id = threadIdx.x + blockIdx.x * blockDim.x;
	T* my_v = &vect[id * LENGTH];

	float rand_num = curand_uniform(&state[id]);

	// random index for swap in range [idx_2, last]
	// simply picking a number in range [0, last] will
	// create bias in the shuffled list
	rand_num *= ((LENGTH - 1) - idx_2) + 0.999999;
	rand_num += idx_2;

	std::size_t idx_1 = (std::size_t)truncf(rand_num);

	// swap current element and random element
	T temp = my_v[idx_1];
	my_v[idx_1] = my_v[idx_2];
	my_v[idx_2] = temp;
}


template <typename T>
__global__ void duplicate_n(T* __restrict vect, const std::size_t LENGTH, const std::size_t N)
{
	std::size_t id = threadIdx.x + blockIdx.x * blockDim.x;
	if (id < LENGTH)
	{
		for (auto i = 1; i < N; i++)
		{
			std::size_t idx = id + i * LENGTH;
			vect[idx] = vect[id];
		}
	}
}


template <typename T>
__global__ void shuffle(T* __restrict vect, const std::size_t LENGTH, curandState* state)
{
	for (std::size_t i = 0; i < LENGTH; i++)
	{
		shuffle_vector(vect, LENGTH, i, state);
	}
}


 void shuffle_wrapper(uint n_blocks, uint n_threads, cudaStream_t& stream, int* __restrict vect, const std::size_t LENGTH, curandState* state)
{
	shuffle<int><<<n_blocks, n_threads, 0, stream>>>(vect, LENGTH, state);
} 


 void duplicate_n_wrapper(uint n_blocks, uint n_threads, cudaStream_t& stream, int* __restrict vect, const std::size_t LENGTH, const std::size_t N)
{
	duplicate_n<int><<<n_blocks, n_threads, 0, stream>>>(vect, LENGTH, N);
}


 void setup_kernel_wrapper(uint n_blocks, uint n_threads, cudaStream_t& stream, curandState *state)
{
	setup_kernel<<<n_blocks, n_threads, 0, stream>>>(state);
}

#endif