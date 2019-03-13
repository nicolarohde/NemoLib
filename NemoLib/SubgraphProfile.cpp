/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

 /*
  * File:   SubgraphProfile.cpp
  * Author: Wooyoung
  *
  * Created on October 29, 2017, 3:05 PM
  */

#include "SubgraphProfile.h"
#include "Utility.hpp"

using std::cout;
using std::ostream;
using std::vector;
using std::unordered_map;

//need the subgraphsize to compute subgraphcount for each label

/* Add subgraphs using label
 * Just to check, we will implement without labeling yet
 */
void SubgraphProfile::add(Subgraph& currentSubgraph, NautyLink& nautylink)
{
	// first, get the label
	graph64 label = nautylink.nautylabel(currentSubgraph);

	// get the current nodes
	vector<vertex>& nodes = currentSubgraph.getNodes();

	// if the key already exists, then update
	if (labelVertexFreqMapMap.count(label) == 0)
	{
		labelVertexFreqMapMap[label] = std::vector<uint64_t>(graphsize, 0);
	}

	// update the map
	for (int i = 0; i < currentSubgraph.getSize(); i++)
	{
		labelVertexFreqMapMap[label][nodes[i]] += 1;
	}
}


unordered_map<graph64, uint64_t> SubgraphProfile::getlabelFreqMap(int subgraphsize)
{
	unordered_map <graph64, uint64_t> labelFreqMap;

	for (auto& p : labelVertexFreqMapMap)
	{
		uint64_t countLabel = get_vector_sum(p.second.begin(), p.second.end(), 0);
		labelFreqMap[p.first] = countLabel / subgraphsize;
	}
	return labelFreqMap;
}

unordered_map <graph64, double> SubgraphProfile::getRelativeFrequencies()
{
	unordered_map<graph64, double> result(labelVertexFreqMapMap.size());
	double totalcount = static_cast<double>(getTotalSubgaphCount());

	for (auto& p : labelVertexFreqMapMap)
	{
		double countLabel = get_vector_sum(p.second.begin(), p.second.end(), 0.0);
		result[p.first] = countLabel / totalcount;
	}

	return result;
}
