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
#include "RandESU.h"
#include "ThreadPool.hpp"	// ThreadPool
#include "Job.hpp"			// Job
#include <numeric>			// reduce
#include <functional>
#include "SubgraphCount.h"
#include <mutex>
#include "Global.hpp"
#if _C17_EXECUTION_AVAILABLE
	#include <execution>	// execution::par_unseq
#endif

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
	static void enumerate(Graph& graph, SubgraphEnumerationResult* subgraphs, int subgraphSize, ThreadPool* my_pool)
	{
		using enum_job = Job<void, Graph&, SubgraphEnumerationResult*, int, const std::vector<double>&, vertex, NautyLink&>;
		void(*rand_esu)(Graph&, SubgraphEnumerationResult*, int, const std::vector<double>&, vertex, NautyLink&) = RandESU::enumerate;

		NautyLink nautylink(subgraphSize, graph.getEdges(), graph.isDirected());

		std::vector<SubgraphCount> all_subgraphs;
		all_subgraphs.reserve(graph.getSize());

		for (std::size_t i = 0; i < graph.getSize(); i++)
		{
			all_subgraphs.push_back(*dynamic_cast<SubgraphCount*>(subgraphs));
			std::vector<double> probs(subgraphSize, 1.0);
			Job_Base* j = new enum_job(rand_esu, graph, &all_subgraphs[i], subgraphSize, probs, i, nautylink);
			my_pool->Add_Job(j);
		} // end for i

		my_pool->Synchronize();

		#if _C17_EXECUTION_AVAILABLE
			*dynamic_cast<SubgraphCount*>(subgraphs) = std::reduce(std::execution::par, all_subgraphs.begin(), all_subgraphs.end(), SubgraphCount());
		#else
			*dynamic_cast<SubgraphCount*>(subgraphs) = std::accumulate(all_subgraphs.begin(), all_subgraphs.end(), SubgraphCount());
		#endif
	} // end method enumerate
};
