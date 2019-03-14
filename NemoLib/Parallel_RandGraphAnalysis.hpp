#pragma once

#include "Config.hpp"
#include "Global.hpp"
#include "RandomGraphAnalysis.h"
#include "SubgraphCount.h"
#include "RandomGraphGenerator.h"
#include "RandESU.h"
#include "Job.hpp"
#include "ThreadPool.hpp"
#include <unordered_map>
#include <vector>

namespace Parallel_Analysis
{
	std::unordered_map <graph64, std::vector<double>> analyze(Graph& targetGraph, std::size_t randomGraphCount, std::size_t subgraphSize, const std::vector<double>& probs, ThreadPool* my_pool)
	{
		using enum_job = Job<void, Graph&, SubgraphEnumerationResult*, int, const std::vector<double>&>;
		using process_job = Job<void, SubgraphCount&>;
		void(*rand_esu)(Graph&, SubgraphEnumerationResult*, int, const std::vector<double>&) = RandESU::enumerate;

		// create the return map and fill it with the labels we found in the
		// target graph, as those are the only labels about which we care
		// TODO consider changing this, as it creates the precondition of 
		// executing the target graph analysis first
		std::unordered_map<graph64, std::vector<double>> labelRelFreqsMap;
		std::vector<SubgraphCount> all_subgraphs(randomGraphCount);

		my_pool->Start_All_Threads();

		std::cout << "Creating jobs for random analysis ..." << std::endl;

		for (int i = 0; i < randomGraphCount; i++)
		{
			//generate random graphs
			Graph randomGraph = std::move(RandomGraphGenerator::generate(targetGraph));

			Job_Base* j = new enum_job(rand_esu, randomGraph, &all_subgraphs[i], subgraphSize, probs);
			
			my_pool->Add_Job(j);
		}

		std::cout << "Waiting for jobs to finish ..." << std::endl;

		my_pool->Synchronize(true);

		for (auto& subgraphCount : all_subgraphs)
		{
			std::unordered_map<graph64, double> curLabelRelFreqMap = std::move(subgraphCount.getRelativeFrequencies());

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
			}
		}

		return labelRelFreqsMap;
	}
}


