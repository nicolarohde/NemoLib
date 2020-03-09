#pragma once

#include <future>                   // packaged_task
#include <unordered_map>            // unordered_map
#include <vector>                   // vector
#include <cstddef>                  // size_t
#include <string>                   // string
      
#include "Config.hpp"               // configuration defines
#include "SubgraphCount.hpp"        // SubgraphCount
#include "RandESU.hpp"              // RandESU
#include "ThreadPool.hpp"           // ThreadPool
#include "RandomGraphGenerator.hpp" // RandomGraphGenerator

namespace Parallel_Analysis
{
	struct AnalyzeArgPack
	{
		AnalyzeArgPack(const Graph* g_, const std::size_t rgc_, const std::size_t sgs_, std::vector<double>& p_, ThreadPool* pool, const std::string& lgp_)
		 : m_graph_target(*g_), m_tp_pool(pool), mu_li_graph_count(rgc_), mu_li_subgraph_size(sgs_), m_vectd_probabilities(p_), m_str_labelg_path(lgp_)
		{}

		const Graph& m_graph_target;

		ThreadPool* m_tp_pool;

		const std::size_t mu_li_graph_count;
		const std::size_t mu_li_subgraph_size;

		std::vector<double>& m_vectd_probabilities;

		const std::string& m_str_labelg_path;
	};

//	std::unordered_map<std::string, std::vector<double>> analyze(const Graph& targetGraph, const std::size_t randomGraphCount, const std::size_t subgraphSize, 
//	                                                             const std::vector<double>& probs, ThreadPool* my_pool, const std::string& labelg_path)
std::unordered_map<std::string, std::vector<double>> analyze(AnalyzeArgPack& args)
	{
		// create the return map and fill it with the labels we found in the
		// target graph, as those are the only labels about which we care
		// TODO consider changing this, as it creates the precondition of
		// executing the target graph analysis first
		std::unordered_map<std::string, std::vector<double>> labelRelFreqsMap;
		std::vector<SubgraphCount> all_subgraphs(args.mu_li_graph_count);

		args.m_tp_pool->Start_All_Threads();

		//std::cout << "Creating jobs for random analysis ..." << std::endl;

		for (std::size_t i{0}; i < args.mu_li_graph_count; i++)
		{
			args.m_tp_pool->Add_Job(
				[&args, &all_subgraphs, i](void) 
				{
					// generate random graphs
					Graph randomGraph = std::move(RandomGraphGenerator::generate(args.m_graph_target));
					SubgraphEnumerationResult* my_subgraphs = dynamic_cast<SubgraphEnumerationResult*>(&all_subgraphs[i]);
					int my_size = static_cast<int>(args.mu_li_subgraph_size);

					RandESU::enumerate(randomGraph, my_subgraphs, my_size, args.m_vectd_probabilities, args.m_str_labelg_path);
				} // end lambda
			); // end Add_Job
		} // end for i

		//std::cout << "Waiting for jobs to finish ..." << std::endl;

		args.m_tp_pool->Synchronize();

		//std::cout << "Jobs completed!" << std::endl;

		for (auto& subgraphCount : all_subgraphs)
		{
			std::unordered_map<std::string, double> curLabelRelFreqMap = std::move(subgraphCount.getRelativeFrequencies());

			// populate labelRelReqsMap with result
			for (const auto& p : curLabelRelFreqMap)
			{
				labelRelFreqsMap[p.first].reserve(args.mu_li_graph_count);
				labelRelFreqsMap[p.first].push_back(p.second);
			} // end for p
		} // end for subgraphCount

		// fill in with zeros any List that is less than subgraph count to
		// ensure non-detection is accounted for.
		for (auto& p : labelRelFreqsMap)
		{
			while (p.second.size() < args.mu_li_graph_count)
			{
				p.second.push_back(0.0);
			} // end while
		} // end for p

		return labelRelFreqsMap;
	} // end method analyze
} // end namespace Parallel_Analysis
