#pragma once

#ifndef __CUDA_SHUFFLER_CUH
#define __CUDA_SHUFFLER_CUH

#include <vector>
#include <cstddef>
#include <string>
#include "device_launch_parameters.h"
#include "curand_kernel.h"

typedef unsigned int uint;

extern void shuffle_wrapper(uint n_blocks, uint n_threads, cudaStream_t& stream, int* __restrict vect, const std::size_t LENGTH, curandState* state);
extern void duplicate_n_wrapper(uint n_blocks, uint n_threads, cudaStream_t& stream, int* __restrict vect, const std::size_t LENGTH, const std::size_t N);
extern void setup_kernel_wrapper(uint n_blocks, uint n_threads, cudaStream_t& stream, curandState *state);


class CUDA_Shuffler
{
public:
	// disallow any kind of move/copy operation to prevent unwanted CUDA side-effects
	CUDA_Shuffler(const CUDA_Shuffler&) = delete;
	CUDA_Shuffler(CUDA_Shuffler&&) = delete;
	CUDA_Shuffler& operator=(const CUDA_Shuffler&) = delete;
	CUDA_Shuffler& operator=(CUDA_Shuffler&&) = delete;


	///<summary>
	/// Default constructor that initializes all class members.
	///</summary>
	///<remarks>
	/// Shuffler will not be ready to do anything until <see cref="CUDA_Shuffler::init"/> is invoked.
	///</remarks>
	CUDA_Shuffler(void) : initialized(false), m_vector_length(0), m_n_vectors(0), m_dev_vertexList(nullptr), last_error(cudaSuccess) {}


	///<summary>
	/// Destructs class object.
	///</summary>
	~CUDA_Shuffler(void) noexcept
	{
		clean_up_CUDA();
	} // end Destructor 


	///<summary>
	/// Initializes the device to begin shuffling and copies <paramref name="vertexList"/> to the device.
	/// On the device <paramref name="vertexList"/> will be replicated <paramref name="N"/> times.
	///</summary>
	///<param name="vertexList">List of integers to shuffle.</param>
	///<param name="N">Number of vectors to shuffle concurrently.</param>
	///<exception>
	///	Any errors that occur will result in immediate return of the function and the member
	/// <see cref="CUDA_Shuffler::last_error"/> will be set to reflect the issue.
	///</exception>
	inline void init(std::vector<int>& vertexList, const std::size_t N)
	{
		if (!initialized)
		{
			last_error = cudaSetDevice(0);

			if (last_error != cudaSuccess)
			{
				std::cerr << "[CUDA Shuffler]: Setting device failed!\n";
				std::cerr << "[CUDA Shuffler]: Error: " << cudaGetErrorString(last_error) << "\n";
				return;
			} // end if

			m_vector_length = vertexList.size();
			m_n_vectors = N;

			last_error = cudaStreamCreate(&compute_stream);

			if (last_error != cudaSuccess)
			{
				std::cerr << "[CUDA Shuffler]: Compute stream creation failed!\n";
				std::cerr << "[CUDA Shuffler]: Error: " << cudaGetErrorString(last_error) << "\n";
				return;
			} // end if

			last_error = cudaStreamCreate(&copy_stream);

			if (last_error != cudaSuccess)
			{
				std::cerr << "[CUDA Shuffler]: Copy stream creation failed!\n";
				std::cerr << "[CUDA Shuffler]: Error: " << cudaGetErrorString(last_error) << "\n";
				return;
			} // end if

			prepare_shuffling(vertexList);
		} // end if		
	} // end method init


	///<summary>
	/// Releases all device memory used by this object and resets device.
	///</summary>
	///<remarks>
	/// Equivalent to invoking destructor.
	///</remarks>
	inline void clean_up() noexcept
	{
		clean_up_CUDA();
	} // end method clean_up


	///<summary>
	/// Initializes the device to begin shuffling and copies <paramref name="vertexList"/> to the device.
	/// On the device <paramref name="vertexList"/> will be replicated the specified number of times.
	///</summary>
	///<param name="vertexList">List of integers to shuffle</param>
	///<exception>
	///	If this function is called prior to <see cref="CUDA_Shuffler::init"/> or after <see cref="CUDA_Shuffler::clean_up"/>
	/// <see cref="CUDA_Shuffler::last_error"/> will be set to <see cref="cudaErrorNotPermitted"/>.
	/// If an error occurs during execution, the error code returned by CUDA will be set to <see cref="CUDA_Shuffler::last_error"/>.
	///</exception>
	inline void shuffle_CUDA(void)
	{
		if (initialized)
		{
			// wait for any host-device and device-host copying to finish
			last_error = cudaStreamSynchronize(copy_stream);

			if (last_error != cudaSuccess)
			{
				std::cerr << "[CUDA Shuffler]: Unable to synchronize copy stream!\n";
				std::cerr << "[CUDA Shuffler]: Error: " << cudaGetErrorString(last_error) << "\n";
				return;
			} // end if

			// schedule shuffling after any scheduled internal copying 
			shuffle_wrapper(1, m_n_vectors, compute_stream, m_dev_vertexList, m_vector_length, m_dev_states);

			last_error = cudaGetLastError();

			if (last_error != cudaSuccess)
			{
				std::cerr << "[CUDA Shuffler]: Unable shuffle matrix!\n";
				std::cerr << "[CUDA Shuffler]: Error: " << cudaGetErrorString(last_error) << "\n";
				return;
			} // end if
		} // end if
		else
		{
			// signify to user that class is not initialized
			last_error = cudaErrorNotPermitted;
		} // else
	} // end method shuffle_CUDA


	///<summary>
	/// Initializes the device to begin shuffling and copies <paramref name="vertexList"/> to the device.
	/// On the device <paramref name="vertexList"/> will be replicated the specified number of times.
	///</summary>
	///<param name="vertexList">List of integers to shuffle</param>
	void get_shuffled_indices(std::vector<std::vector<int>>& shuffled_vertexLists)
	{
		const std::size_t n_bytes = m_vector_length * sizeof(int);

		// wait for shuffling to complete
		last_error = cudaStreamSynchronize(compute_stream);

		if (last_error != cudaSuccess)
		{
			std::cerr << "[CUDA Shuffler]: Unable to synchronize compute stream!\n";
			std::cerr << "[CUDA Shuffler]: Error: " << cudaGetErrorString(last_error) << "\n";
			return;
		} // end if

		for (auto i = 0; i < m_n_vectors; i++)
		{
			last_error = cudaMemcpyAsync(shuffled_vertexLists[i].data(), m_dev_vertexList + (i * m_vector_length), n_bytes, cudaMemcpyDeviceToHost, copy_stream);
			if (last_error != cudaSuccess)
			{
				std::cerr << "[CUDA Shuffler]: Unable to schedule memory transfer from device to host!\n";
				std::cerr << "[CUDA Shuffler]: Error: " << cudaGetErrorString(last_error) << "\n";
				return;
			} // end if
		} // end for i

		// wait for all transfers to complete
		last_error = cudaStreamSynchronize(copy_stream);

		if (last_error != cudaSuccess)
		{
			std::cerr << "[CUDA Shuffler]: Unable to synchronize copy stream!\n";
			std::cerr << "[CUDA Shuffler]: Error: " << cudaGetErrorString(last_error) << "\n";
			return;
		} // end if
	} // end method get_shuffled_indices


	///<summary>
	/// Returns the last error code generated.
	///</summary>
	inline cudaError_t get_last_error(void) noexcept
	{
		return last_error;
	} // end get_last_error

private:
	///<summary> cuRAND RNG states. </summary>
	curandState* m_dev_states;

	///<summary> Execution stream for memory transfers. </summary>
	cudaStream_t copy_stream;
	///<summary> Execution stream for computations. </summary>
	cudaStream_t compute_stream;

	cudaError_t last_error;

	///<summary> Number of rows in device matrix. </summary>
	std::size_t m_n_vectors;
	///<summary> Number of elements in each row. </summary>
	std::size_t m_vector_length;

	///<summary> Device memory. </summary>
	int* m_dev_vertexList;

	///<summary> Whether or not memory was allocated. </summary>
	bool initialized;


	///<summary>
	/// Initializes the device to begin shuffling and copies <paramref name="vertexList"/> to the device.
	/// On the device <paramref name="vertexList"/> will be replicated the specified number of times.
	///</summary>
	///<param name="vertexList">List of integers to shuffle</param>
	void prepare_shuffling(std::vector<int>& vertexList)
	{
		constexpr std::size_t N_THREADS = 256;
		const std::size_t n_bytes = vertexList.size() * sizeof(int);

		// alloc memory for N copies of vertex list on device
		last_error = cudaMalloc((void**)&m_dev_vertexList, n_bytes * m_n_vectors);

		if (last_error != cudaSuccess)
		{
			std::cerr << "[CUDA Shuffler]: Unable allocate device matrix!\n";
			std::cerr << "[CUDA Shuffler]: Error: " << cudaGetErrorString(last_error) << "\n";
			cudaFree(m_dev_vertexList);
			return;
		} // end if

		// signify that at least some memory has been allocated
		initialized = true;

		// alloc memory for 
		last_error = cudaMalloc((void**)&m_dev_states, sizeof(curandState) * m_n_vectors);

		if (last_error != cudaSuccess)
		{
			std::cerr << "[CUDA Shuffler]: Unable allocate device cuRAND states!\n";
			std::cerr << "[CUDA Shuffler]: Error: " << cudaGetErrorString(last_error) << "\n";
			cudaFree(m_dev_states);
			return;
		} // end if

		// transfer memory to device
		last_error = cudaMemcpyAsync(m_dev_vertexList, vertexList.data(), n_bytes, cudaMemcpyHostToDevice, copy_stream);

		if (last_error != cudaSuccess)
		{
			std::cerr << "[CUDA Shuffler]: Unable copy vertex list to device!\n";
			std::cerr << "[CUDA Shuffler]: Error: " << cudaGetErrorString(last_error) << "\n";
			return;
		} // end if

		setup_kernel_wrapper(1, m_n_vectors, compute_stream, m_dev_states);
		last_error = cudaGetLastError();

		if (last_error != cudaSuccess)
		{
			std::cerr << "[CUDA Shuffler]: Unable initialize cuRAND states!\n";
			std::cerr << "[CUDA Shuffler]: Error: " << cudaGetErrorString(last_error) << "\n";
			return;
		} // end if

		last_error = cudaStreamSynchronize(copy_stream);

		if (last_error != cudaSuccess)
		{
			std::cerr << "[CUDA Shuffler]: Unable to synchronize copy stream!\n";
			std::cerr << "[CUDA Shuffler]: Error: " << cudaGetErrorString(last_error) << "\n";
			return;
		} // end if

		duplicate_n_wrapper((m_vector_length + N_THREADS - 1) / N_THREADS, N_THREADS, compute_stream, m_dev_vertexList, m_vector_length, m_n_vectors);
		last_error = cudaGetLastError();

		if (last_error != cudaSuccess)
		{
			std::cerr << "[CUDA Shuffler]: Duplicating input vector on device failed!\n";
			std::cerr << "[CUDA Shuffler]: Error: " << cudaGetErrorString(last_error) << "\n";
			return;
		} // end if
	}


	///<summary>
	/// Releases all device memory used by this object and resets device.
	///</summary>
	inline void clean_up_CUDA(void) noexcept
	{
		if (initialized)
		{
			initialized = false;
			cudaDeviceSynchronize();
			cudaFree(m_dev_states);
			cudaFree(m_dev_vertexList);
			cudaStreamDestroy(copy_stream);
			cudaStreamDestroy(compute_stream);
			cudaDeviceReset();
		} // end if 		
	} // end method clean_up_CUDA
};

#endif