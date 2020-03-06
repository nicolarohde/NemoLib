/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   SubgraphProfile.h
 * Author: Wooyoung
 *
 * Created on October 29, 2017, 3:05 PM
 */

#ifndef SUBGRAPHPROFILE_H
#define SUBGRAPHPROFILE_H

#include "Config.hpp"
#include "SubgraphEnumerationResult.h"
#include "Utility.hpp"
#include "graph64.hpp"

class Subgraph;
class NautyLink;

// The primary structure of a subgraph profile. Essentially a table to map
	// labels(String) and nodes(Integer) to the frequency of subgraphs
	// of type label that include the node.

/* This class will have two maps: subgraphprofile and subgraphcount map */
class SubgraphProfile : public SubgraphEnumerationResult 
{
public:
    // get the size of vertex as parameter
	SubgraphProfile(uint64_t size) : graphsize(size) {}
    virtual ~SubgraphProfile() = default;
    virtual void add(Subgraph&, NautyLink&);

	std::unordered_map<std::string, uint64_t> getlabelFreqMap(int); //need subgraphsize to calculate frequency
	std::unordered_map <std::string, double> getRelativeFrequencies();

	inline std::unordered_map<std::string, std::vector<uint64_t>> getlabelVertexFreqMapMap()
	{
		return labelVertexFreqMapMap;
	}

private:
	std::unordered_map<std::string, std::vector<uint64_t>> labelVertexFreqMapMap;
	uint64_t graphsize;

	inline uint64_t getTotalSubgaphCount(void)
	{
		uint64_t totalcount = 0;
		for (auto& p : labelVertexFreqMapMap)
		{
			totalcount += get_vector_sum<std::vector<uint64_t>::iterator, uint64_t>(p.second.begin(), p.second.end(), 0);
		}
		return totalcount;
	}
};

#endif /* SUBGRAPHPROFILE_H */
