#pragma once


/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

 /*
  * File:   ESU.h
  * Author: Wooyoung
  *
  * Created on October 25, 2017, 1:06 PM
  */
#include "Config.hpp"
#include "Utility.hpp"
#include "RandESU.hpp"
#include "ThreadPool.hpp"	// ThreadPool
#include "SubgraphCount.hpp"
#include <functional>


  /**
   * ESU is a static class used for executing the Enumerate Subgraphs algorithm
   * on a network graph.
   */
namespace ESU_Parallel
{
	/**
	  * Enumerates Subgraphs using the ESU algorithm. Requires user to specify
	  * return type(s) and provide the accompanying data structures.
	  *
	  * @param graph the graph on which to execute ESU
	  * @param subgraphs the SubgraphEnumerationResult into which to enumerated
	  *                  Subgraphs will be stored.
	  * @param subgraphSize the size of the target Subgraphs
	  */
	static void enumerate(Graph& graph, SubgraphEnumerationResult* subgraphs, int subgraphSize, ThreadPool* my_pool, const std::string& labelg_path)
	{
		// std::cerr << "Creating nautylink object ..." << std::endl;
		NautyLink nautylink(labelg_path, subgraphSize, graph.getEdges(), graph.isDirected());

		std::vector<SubgraphCount> all_subgraphs;
		all_subgraphs.reserve(graph.getSize());

		// std::cerr << "Creating jobs ..." << std::endl;

		for (std::size_t i = 0; i < graph.getSize(); i++)
		{
			all_subgraphs.push_back(*dynamic_cast<SubgraphCount*>(subgraphs));

			int my_size = subgraphSize;
			vertex my_vertex = static_cast<vertex>(i);

			my_pool->Add_Job(
				[&graph, &all_subgraphs, i, my_size, my_vertex, &nautylink](void)
				{
					const std::vector<double> probs(my_size, 1.0);
					Graph& my_graph = graph;
					SubgraphEnumerationResult* my_result = &all_subgraphs[i];
					NautyLink& my_link = nautylink;
					//enumerate(Graph&, SubgraphEnumerationResult*, int, const std::vector<double>&, vertex, NautyLink&);
					RandESU::enumerate(my_graph, my_result, my_size, probs, my_vertex, my_link);
				} // end lambda
			); // end Add_Job
		} // end for i

		// std::cerr << "Synchronizing ..." << std::endl;

		my_pool->Synchronize();

		// std::cerr << "Done with sync ..." << std::endl;

		*dynamic_cast<SubgraphCount*>(subgraphs) = get_vector_sum(all_subgraphs.begin(), all_subgraphs.end(), SubgraphCount());
	} // end method enumerate
};
