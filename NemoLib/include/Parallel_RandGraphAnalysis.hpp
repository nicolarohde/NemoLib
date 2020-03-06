#pragma once

#include <future>
#include <unordered_map>
#include <vector>

#include "Config.hpp"
#include "Global.hpp"
#include "graph64.hpp"
#include "SubgraphCount.hpp"
#include "RandESU.h"
#include "ThreadPool.hpp"

#if _USE_CUDA
	#include "CUDA_RandomGraphGenerator.hpp"
#else
	#include "RandomGraphGenerator.h"
#endif

namespace Parallel_Analysis
{
#if _USE_CUDA

	std::unordered_map <std::string, std::vector<double>> analyze(Graph& targetGraph, std::size_t randomGraphCount, std::size_t subgraphSize, const std::vector<double>& probs, ThreadPool* my_pool)
	{
		using enum_job = Job<void, Graph&, SubgraphEnumerationResult*, int, const std::vector<double>&>;
		using process_job = Job<void, SubgraphCount&>;
		void(*rand_esu)(Graph&, SubgraphEnumerationResult*, int, const std::vector<double>&) = RandESU::enumerate;

		// create the return map and fill it with the labels we found in the
		// target graph, as those are the only labels about which we care
		// TODO consider changing this, as it creates the precondition of
		// executing the target graph analysis first
		std::unordered_map<std::string, std::vector<double>> labelRelFreqsMap;
		std::vector<SubgraphCount> all_subgraphs(randomGraphCount);

		my_pool->Start_All_Threads();

		std::cout << "Creating jobs for random analysis ..." << std::endl;

		std::vector<Graph> random_graphs;

		auto RGG_thread = std::thread([&](void) {CUDA_RandomGraphGenerator::generate(targetGraph, random_graphs, randomGraphCount); });

		for (int i = 0; i < randomGraphCount; i++)
		{
			while (random_graphs.size() < (i + 1))
			{
				std::this_thread::yield();
			} // end while

			Job_Base* j = new enum_job(rand_esu, random_graphs[i], &all_subgraphs[i], subgraphSize, probs);
			my_pool->Add_Job(j);
		} // end for i

		if (RGG_thread.joinable())
		{
			RGG_thread.join();
		} // end if

		std::cout << "Waiting for jobs to finish ..." << std::endl;

		my_pool->Synchronize(false);

		for (auto& subgraphCount : all_subgraphs)
		{
			std::unordered_map<std::string, double> curLabelRelFreqMap = std::move(subgraphCount.getRelativeFrequencies());

			// populate labelRelReqsMap with result
			for (const auto& p : curLabelRelFreqMap)
			{
				labelRelFreqsMap[p.first].reserve(randomGraphCount);
				labelRelFreqsMap[p.first].push_back(p.second);
			} // end for p
		} // end for subgraphCount

		// fill in with zeros any List that is less than subgraph count to
		// ensure non-detection is accounted for.
		for (auto& p : labelRelFreqsMap)
		{
			while (p.second.size() < randomGraphCount)
			{
				p.second.push_back(0.0);
			} // end while
		} // end for p

		return labelRelFreqsMap;
	} // end method analyze

#else

	std::unordered_map<std::string, std::vector<double>> analyze(Graph& targetGraph, std::size_t randomGraphCount, std::size_t subgraphSize, 
	                                                          const std::vector<double>& probs, ThreadPool* my_pool, const std::string& labelg_path)
	{
		// create the return map and fill it with the labels we found in the
		// target graph, as those are the only labels about which we care
		// TODO consider changing this, as it creates the precondition of
		// executing the target graph analysis first
		std::unordered_map<std::string, std::vector<double>> labelRelFreqsMap;
		std::vector<SubgraphCount> all_subgraphs(randomGraphCount);

		my_pool->Start_All_Threads();

		//std::cout << "Creating jobs for random analysis ..." << std::endl;

		for (int i = 0; i < randomGraphCount; i++)
		{
			my_pool->Add_Job(
				[&targetGraph, &all_subgraphs, i, subgraphSize, &probs, labelg_path](void) 
				{
					// generate random graphs
					Graph randomGraph = std::move(RandomGraphGenerator::generate(targetGraph));
					SubgraphEnumerationResult* my_subgraphs = dynamic_cast<SubgraphEnumerationResult*>(&all_subgraphs[i]);
					int my_size = static_cast<int>(subgraphSize);

					RandESU::enumerate(randomGraph, my_subgraphs, my_size, probs, labelg_path);
				} // end lambda
			); // end Add_Job
		} // end for i

		//std::cout << "Waiting for jobs to finish ..." << std::endl;

		my_pool->Synchronize();

		//std::cout << "Jobs completed!" << std::endl;

		for (auto& subgraphCount : all_subgraphs)
		{
			std::unordered_map<std::string, double> curLabelRelFreqMap = std::move(subgraphCount.getRelativeFrequencies());

			// populate labelRelReqsMap with result
			for (const auto& p : curLabelRelFreqMap)
			{
				labelRelFreqsMap[p.first].reserve(randomGraphCount);
				labelRelFreqsMap[p.first].push_back(p.second);
			} // end for p
		} // end for subgraphCount

		// fill in with zeros any List that is less than subgraph count to
		// ensure non-detection is accounted for.
		for (auto& p : labelRelFreqsMap)
		{
			while (p.second.size() < randomGraphCount)
			{
				p.second.push_back(0.0);
			} // end while
		} // end for p

		return labelRelFreqsMap;
	} // end method analyze

#endif

} // end namespace Parallel_Analysis
