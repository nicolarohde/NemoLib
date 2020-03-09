/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

 /*
  * File:   SubgraphCount.h
  * Author: Wooyoung
  *
  * Created on October 18, 2017, 4:39 PM
  */

#ifndef SUBGRAPHCOUNT_H
#define SUBGRAPHCOUNT_H
#include "Config.hpp"
#include "SubgraphEnumerationResult.hpp"
#include "Subgraph.hpp"
#include "NautyLink.hpp"

  /**
   * Representation of a Subgraph Count structure, which stores the number of
   * subgraphs detected in a network separated by subgraph type. A SubgraphCount
   * object exists in one of two states: labeled and unlabeled. Certain
   * operations can only be performed based on the Subgraph's labeled state.
   */
class SubgraphCount : public SubgraphEnumerationResult
{
public:
	/**
	 * Construct an empty SubgraphCount. Create a sparse_hash_map (like a hash map)
	 */
	SubgraphCount() = default;
	virtual ~SubgraphCount() = default;
	SubgraphCount(const SubgraphCount&) = default;
	SubgraphCount(SubgraphCount&&) = default;
	SubgraphCount& operator=(const SubgraphCount&) = default;
	SubgraphCount& operator=(SubgraphCount&&) = default;


	virtual std::unordered_map<std::string, double> getRelativeFrequencies() const
	{
		std::unordered_map<std::string, double> result_map(labelFreqMap.size());
		uint64_t totalSubgraphCount = 0;

		for (const auto& p : labelFreqMap)
		{
			totalSubgraphCount += p.second;
		}

		for (const auto& p : labelFreqMap)
		{
			double count = static_cast<double>(p.second);
			result_map[p.first] = count / static_cast<double>(totalSubgraphCount);
		}

		return result_map;
	}


	/* Implement the add function of subgraph enumeration result*/
	inline virtual void add(Subgraph& currentSubgraph, NautyLink& nautylink)
	{
		//std::cerr << "In SubgraphCount::add" << std::endl;

		std::string label = nautylink.nautylabel_helper(currentSubgraph);
		add(currentSubgraph, nautylink, label);
	}

	/* Implement the add function of subgraph enumeration result*/
	inline virtual void add(Subgraph& currentSubgraph, NautyLink& nautylink, const std::string& label)
	{
		uint64_t total = (labelFreqMap.count(label) == 0 ? 1 : labelFreqMap[label] + 1);
		labelFreqMap[label] = total;
	}


	inline std::unordered_map<std::string, uint64_t> getlabelFreqMap() const
	{
		return labelFreqMap;
	}


	inline std::unordered_map<std::string, uint64_t>* getLabelFreqMapAccess()
	{
		return &labelFreqMap;
	}


	inline SubgraphCount operator+(const SubgraphCount& RHS)
	{
		SubgraphCount out(*this);
		out += RHS;
		return out;
	}


	inline SubgraphCount& operator+=(const SubgraphCount& RHS)
	{
		for (auto& p : RHS.labelFreqMap)
		{
			if (labelFreqMap.count(p.first) == 0)
			{
				labelFreqMap[p.first] = p.second;
			}
			else
			{
				labelFreqMap[p.first] += p.second;
			}
		}
		return *this;
	}


	inline std::size_t size(void) const noexcept
	{
		return labelFreqMap.size();
	}


	inline void output() const noexcept
	{
		for (const auto& x : labelFreqMap)
		{
			std::cout << x.first << " -> " << x.second << std::endl;
		}
	}

protected:
	std::unordered_map<std::string, uint64_t> labelFreqMap;
};

#endif /* SUBGRAPHCOUNT_H */