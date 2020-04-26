#pragma once

#include <future>                   // packaged_task
#include <unordered_map>            // unordered_map
#include <vector>                   // vector
#include <cstddef>                  // size_t
#include <string>                   // string
      
#include "Config.hpp"               // configuration defines
#include "SubgraphCount.hpp"        // SubgraphCount
#include "ESU_Parallel.hpp"
#include "ThreadPool.hpp"           // ThreadPool
#include "RandomGraphGenerator.hpp" // RandomGraphGenerator
#include "Logger.hpp"

#include "loguru.hpp"

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


std::unordered_map<std::string, std::vector<double>> analyze(AnalyzeArgPack& args)
	{
		// create the return map and fill it with the labels we found in the
		// target graph, as those are the only labels about which we care
		// TODO consider changing this, as it creates the precondition of
		// executing the target graph analysis first
		std::unordered_map<std::string, std::vector<double>> labelRelFreqsMap;
		std::vector<SubgraphCount> all_subgraphs(args.mu_li_graph_count);

		for (std::size_t i{0}; i < args.mu_li_graph_count; i++)
		{
			if (0 == i % 10) 
            {
                LOG_F(INFO, "Working on random graph %uz / %uz", i + 1, args.mu_li_graph_count);
            }

			// generate random graph
			Graph randomGraph = std::move(RandomGraphGenerator::generate(args.m_graph_target));

			ESU_Parallel::enumerate<SubgraphCount>(randomGraph, &all_subgraphs[i], static_cast<int>(args.mu_li_subgraph_size), args.m_tp_pool, args.m_str_labelg_path);
		} // end for i

        LOG_F(INFO, "Merging results ...");

		for (auto& subgraphCount : all_subgraphs)
		{
			std::unordered_map<std::string, double> curLabelRelFreqMap = std::move(subgraphCount.getRelativeFrequencies());

			// populate labelRelReqsMap with result
			for (const auto& p : curLabelRelFreqMap)
			{
				auto* v = &labelRelFreqsMap[p.first];
				v->reserve(args.mu_li_graph_count);
				v->push_back(p.second);
			} // end for p
		} // end for subgraphCount

		// fill in with zeros any List that is less than subgraph count to
		// ensure non-detection is accounted for.
		for (auto& p : labelRelFreqsMap)
		{
			if (p.second.size() < args.mu_li_graph_count)
			{
				p.second.resize(args.mu_li_graph_count, 0.0);
			} // end while
		} // end for p

		return labelRelFreqsMap;
	} // end method analyze
} // end namespace Parallel_Analysis
