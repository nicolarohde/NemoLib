/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

 /*
  * File:   RandomGraphAnalysis.cpp
  * Author: Wooyoung
  *
  * Created on November 3, 2017, 1:29 PM
  */

  /**
   * RandomGraphAnalysis is a facade class that generates and enumerates using
   * RAND-ESU a set of random network graphs based on the degree sequence vector
   * of a specified graph. The output of the analyze() method can be used by a
   * StatisticalAnalysis object to determine whether a target graph contains any
   * network motifs.
   */

#include "RandomGraphAnalysis.hpp"
#include "SubgraphCount.hpp"
#include "Graph.hpp"
#include "RandomGraphGenerator.hpp"
#include "RandESU.hpp"

#include "loguru.hpp"

using std::vector;
using std::unordered_map;

/**
 * Generate and enumerate a set of random graphs.
 * @param targetGraph the network graph from which to derive a degree
 *                    sequence vector for generating random graphs
 * @param randomGraphCount the number of random graphs to use for analysis
 * @param subgraphSize the size of subgraph to enumerate
 * @param probs the probability vector to be used by the RAND-ESU algorithm
 * @return mapping of labels to relative frequencies as found in the
 * random graph pool
 */


unordered_map <std::string, vector<double>> RandomGraphAnalysis::analyze(Graph& targetGraph, int randomGraphCount, int subgraphSize, const vector<double>& probs, const std::string& labelg_path)
{
	// create the return map and fill it with the labels we found in the
	// target graph, as those are the only labels about which we care
	// TODO consider changing this, as it creates the precondition of
	// executing the target graph analysis first
	unordered_map<std::string, vector<double>> labelRelFreqsMap;
	for (int i = 0; i < randomGraphCount; i++)
	{
        LOG_F(INFO, "Analyzing random graph %i", i + 1);

		//generate random graphs
		Graph randomGraph = std::move(RandomGraphGenerator::generate(targetGraph));

		// enumerate random graphs
		SubgraphCount subgraphCount;
		RandESU::enumerate<SubgraphCount>(randomGraph, &subgraphCount, subgraphSize, probs, labelg_path);
		unordered_map<std::string, double> curLabelRelFreqMap = std::move(subgraphCount.getRelativeFrequencies());

		// populate labelRelReqsMap with result
		for (const auto& curLabelRelFreqPair : curLabelRelFreqMap)
		{
			std::string curLabel = curLabelRelFreqPair.first;
			double curFreq = curLabelRelFreqPair.second;
			labelRelFreqsMap[curLabel].push_back(curFreq);
		}
	}

	// fill in with zeros any List that is less than subgraph count to
	// ensure non-detection is accounted for.
	for (auto& p : labelRelFreqsMap)
	{
		while (p.second.size() < static_cast<std::size_t>(randomGraphCount))
		{
			p.second.push_back(0.0);
		}
	}

	return labelRelFreqsMap;
}
