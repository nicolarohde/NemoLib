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
#include <chrono>

#include "loguru.hpp"       // DLOG_F, LOG_F


  /**
   * ESU is a static class used for executing the Enumerate Subgraphs algorithm
   * on a network graph.
   */
namespace ESU_Parallel
{
	static constexpr std::size_t BATCH_SIZE = 10000;

	template <typename T>
	static void accumulate_subgraphs(const std::vector<T>& vect_subgraphs, T* p_ser_subgraphs)
	{
		for (const auto& subgraph : vect_subgraphs)
		{
			*(dynamic_cast<T*>(p_ser_subgraphs)) += subgraph;
		} // end for
	} // end for


	/**
	  * Enumerates Subgraphs using the ESU algorithm. Requires user to specify
	  * return type(s) and provide the accompanying data structures.
	  *
	  * @param graph the graph on which to execute ESU
	  * @param subgraphs the SubgraphEnumerationResult into which to enumerated
	  *                  Subgraphs will be stored.
	  * @param subgraphSize the size of the target Subgraphs
	  */
	template <typename T>
	static void enumerate(Graph& graph, T* subgraphs, int subgraphSize, ThreadPool* my_pool, const std::string& labelg_path)
	{
        DLOG_F(DEBUG_LEVEL, "In ESU_Parallel::enumerate");

		NautyLink nautylink(labelg_path, subgraphSize, graph.getEdges(), graph.isDirected());

		auto n_jobs = graph.getSize();

		my_pool->Start_All_Threads();

		// process the batch's data
		for (std::size_t i{0}; i < n_jobs; i++)
		{
			if (0 == i % 10000)
			{
                LOG_F(INFO, "Creating job %zu / %zu", i+1, n_jobs);
			}

			my_pool->Add_Job(
				[&graph, &subgraphs, i, subgraphSize, &nautylink](void)
				{
					const std::vector<double> probs(subgraphSize, 1.0);
					Graph& my_graph = graph;
					T* my_result = subgraphs;
					NautyLink& my_link = nautylink;
					RandESU::enumerate<T>(my_graph, my_result, subgraphSize, probs, static_cast<vertex>(i), my_link);
				} // end lambda
			); // end Add_Job
		} // end for i

        LOG_F(INFO, "Waiting for enumeration to finish ...");

		my_pool->Synchronize();

        LOG_F(INFO, "Enumeration done");
	} // end method enumerate
};
