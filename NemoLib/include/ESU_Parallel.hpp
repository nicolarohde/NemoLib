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
		//{Logger() << "In ESU_Parallel::enumerate" << std::endl;}
		NautyLink nautylink(labelg_path, subgraphSize, graph.getEdges(), graph.isDirected());

		auto n_jobs = graph.getSize();

		my_pool->Start_All_Threads();

		// process the batch's data
		for (std::size_t i{0}; i < n_jobs; i++)
		{
			if (0 == i % 10000)
			{
				Logger() << "Creating job " << i + 1 << "/" << n_jobs << std::endl;
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

		{Logger() << "Synchronizing ..." << std::endl;}		

		my_pool->Synchronize();

		{Logger() << "Synchronized" << std::endl;}
	} // end method enumerate


	// template <typename T>
	// static void enumerate(Graph& graph, T* subgraphs, int subgraphSize, ThreadPool* my_pool, const std::string& labelg_path)
	// {
	// 	NautyLink nautylink(labelg_path, subgraphSize, graph.getEdges(), graph.isDirected());

	// 	std::vector<T> all_subgraphs;

	// 	T my_subgraphs = *subgraphs;

	// 	auto n_jobs = graph.getSize();
	// 	auto n_batches = static_cast<std::size_t>(std::ceil(static_cast<double>(n_jobs) / static_cast<double>(BATCH_SIZE)));

	// 	all_subgraphs.reserve(std::min(n_jobs, BATCH_SIZE));

	// 	for (std::size_t batch{0}; batch < n_batches; batch++)
	// 	{
	// 		// the number of jobs in this batch to account for uneven splits
	// 		std::size_t n_remaining = (n_jobs - BATCH_SIZE * batch);
	// 		std::size_t cur_jobs = std::min(n_remaining, BATCH_SIZE);

	// 		//std::cerr << "Processing batch " << batch + 1 << "/" << n_batches << std::endl;
	// 		//std::cerr << "Jobs in this batch " << cur_jobs << std::endl;
	// 		put_time_stamp(std::cerr) << "Processing batch " << batch + 1 << "/" << n_batches << std::endl;
	// 		put_time_stamp(std::cerr) << "Jobs in this batch " << cur_jobs << std::endl;

	// 		// process the batch's data
	// 		for (std::size_t i{0}; i < cur_jobs; i++)
	// 		{
	// 			//std::cerr << "Pushing back subgraph ..." << std::endl;
	// 			all_subgraphs.push_back(my_subgraphs);
	// 			//std::cerr << "Done pushing back subgraph" << std::endl;

	// 			//int my_size = subgraphSize;
	// 			//vertex my_vertex = static_cast<vertex>(i);

	// 			//std::cerr << "Adding job ..." << std::endl;
	// 			my_pool->Add_Job(
	// 				[&graph, &all_subgraphs, i, subgraphSize, &nautylink](void)
	// 				{
	// 					const std::vector<double> probs(subgraphSize, 1.0);
	// 					Graph& my_graph = graph;
	// 					T* my_result = &all_subgraphs[i];
	// 					NautyLink& my_link = nautylink;
	// 					//enumerate(Graph&, SubgraphEnumerationResult*, int, const std::vector<double>&, vertex, NautyLink&);
	// 					RandESU::enumerate<T>(my_graph, my_result, subgraphSize, probs, static_cast<vertex>(i), my_link);
	// 				} // end lambda
	// 			); // end Add_Job
	// 		} // end for i


	// 		put_time_stamp(std::cerr) << "Synchronizing ..." << std::endl;

	// 		my_pool->Synchronize();

	// 		put_time_stamp(std::cerr) << "Done with sync" << std::endl;
	// 		put_time_stamp(std::cerr) << "All subgraphs length: " << all_subgraphs.size() << std::endl;
	// 		put_time_stamp(std::cerr) << "Merging subgraphs ... " << std::endl;

	// 		// method 1 (19m 2s): 
	// 		//*subgraphs += get_vector_sum(all_subgraphs.begin(), all_subgraphs.end(), T());

	// 		// method 2 (20m 20s):
	// 		//*subgraphs = std::move(std::accumulate(all_subgraphs.begin(), all_subgraphs.end(), *subgraphs));

	// 		// method 3 (TBD min):
	// 		accumulate_subgraphs<T>(all_subgraphs, subgraphs);

	// 		put_time_stamp(std::cerr) << "Done with accumulation, deleting batch temporary data ..." << std::endl;

	// 		all_subgraphs.clear();
	// 	} // end for batch

	// 	//std::cerr << "Synchronizing ..." << std::endl;
	// 	//my_pool->Synchronize();
	// 	//std::cerr << "Done with sync ..." << std::endl;
	// 	//std::cerr << "All subgraphs length: " << all_subgraphs.size() << std::endl;
	// 	//*dynamic_cast<T*>(subgraphs) = get_vector_sum(all_subgraphs.begin(), all_subgraphs.end(), T());
	// } // end method enumerate
};
