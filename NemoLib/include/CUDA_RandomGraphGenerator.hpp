#pragma once

#ifndef __CUDA_RGG_HPP
#define __CUDA_RGG_HPP

#include "Config.hpp"

#if _USE_CUDA

#include <vector>							// vector
#include <cstddef>						// size_t
#include <thread>							// thread
#include "Graph.hpp"						// Graph
#include "Utility.hpp"				// get_vector_sum
#include "CUDA_Shuffler.hpp"	// CUDA_Shuffler

class CUDA_RandomGraphGenerator
{
public:
	CUDA_RandomGraphGenerator() = delete;

	static void generate(Graph& g, std::vector<Graph>& random_graphs, std::size_t N)
	{
		auto dir = g.isDirected();
		auto _make_graphs = std::bind(make_graphs, std::placeholders::_1, std::placeholders::_2, dir);
		auto _init = std::bind(init_wrapper, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

		// in the future this should be dependent on device memory
		// available on the GPU doing the shuffling
		const std::size_t shuffle_batch_size = 320; // number of graphs to build simultaneously
		const std::size_t num_batches = (N + shuffle_batch_size - 1) / shuffle_batch_size;
		std::vector<int> vertexList;

		CUDA_Shuffler shuffler;

		// get our vertex list
		graph_to_vertex_list(vertexList, g);

		// setup CUDA device and memory
		//auto setup_cuda = std::thread(_init, &shuffler, vertexList, shuffle_batch_size);
		auto setup_cuda = std::thread([&](void) {shuffler.init(vertexList, shuffle_batch_size); });
		//init_wrapper(&shuffler, vertexList, shuffle_batch_size);

		// reserve memoy while waiting for CUDA setup
		std::vector<std::vector<int>> shuffled_lists(shuffle_batch_size, std::vector<int>(vertexList.size(), 0));
		random_graphs.reserve(N);

		if (setup_cuda.joinable())
		{
			setup_cuda.join();
		} // end if

		std::thread graph_maker;

		for (auto i = 0; i < num_batches; i++)
		{
			shuffler.shuffle_CUDA();

			if (i > 0 && graph_maker.joinable())
			{
				graph_maker.join();
			} // end if

			shuffler.get_shuffled_indices(shuffled_lists);
			//graph_maker = std::thread(_make_graphs, random_graphs, shuffled_lists);
			graph_maker = std::thread([&](void) {_make_graphs(random_graphs, shuffled_lists); });
			//_make_graphs(random_graphs, shuffled_lists);
		} // end for i

		if (graph_maker.joinable())
		{
			graph_maker.join();
		} // end if

		// release CUDA memory and reset device
		shuffler.clean_up();
	} // end method generate

protected:

	static void graph_to_vertex_list(std::vector<int>& vertexList, Graph& g)
	{
		std::vector <int> degreeSeq(g.getSize(), 0);

		for (int currentVertex = 0; currentVertex < g.getSize(); currentVertex++)
		{
			degreeSeq[currentVertex] = g.getAdjacencyList(currentVertex).size();
		} // end for currentVertex

		// reserve memory for all vertices
		vertexList.reserve(get_vector_sum(degreeSeq.begin(), degreeSeq.end(), 0));

		// generate randomized list of vertices
		// the vertexList is a set where each node is represented by a number
		// of elements equal to that vertex's degree
		for (int vertex = 0; vertex < g.getSize(); vertex++)
		{
			for (int degree = 0; degree < degreeSeq[vertex]; degree++)
			{
				vertexList.push_back(vertex);
			} // end for degree
		} // end for vertex
	} // end method graph_to_vertex_list


	static void init_wrapper(CUDA_Shuffler* shuffler, std::vector<int>& vertexList, const std::size_t shuffle_batch_size)
	{
		shuffler->init(vertexList, shuffle_batch_size);
	} // end method init_wrapper


	static void make_graphs(std::vector<Graph>& random_graphs, const std::vector<std::vector<int>> vertexLists, const bool directed)
	{
		for (auto& v : vertexLists)
		{
			random_graphs.push_back(Graph(v, (v.size() / 2), directed));
		} // end for v
	} // end method make_graphs

};

#endif	/* end if _USE_CUDA */
#endif
