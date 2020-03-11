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

#include <mutex>

#include "Utility.hpp"
#include "Config.hpp"
#include "SubgraphEnumerationResult.hpp"
#include "Subgraph.hpp"
#include "NautyLink.hpp"
#include "Logger.hpp"

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

	SubgraphCount(const SubgraphCount& OTHER)
	{
		*this = OTHER;
	} // end Copy Constructor

	SubgraphCount(SubgraphCount&& other) 
	{
		*this = std::move(other);
	} // end Move Constructor

	SubgraphCount& operator=(const SubgraphCount& OTHER)
	{
		labelFreqMap = OTHER.labelFreqMap;
		return *this;
	} // end Copy Assignment
	SubgraphCount& operator=(SubgraphCount&& other)
	{
		labelFreqMap = std::move(other.labelFreqMap);
		return *this;
	} // end Move Assignment


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
	virtual void add(Subgraph& currentSubgraph, NautyLink& nautylink)
	{
		//put_time_stamp(std::cerr) << " [Thread: " << std::this_thread::get_id() << "]: " << "In SubgraphCount::add(2)" << std::endl;
		//{Logger()  << "[Thread: " << std::this_thread::get_id() << "]: " << "In SubgraphCount::add(2)" << std::endl;}
		
		std::string label{std::move(nautylink.nautylabel_helper(currentSubgraph))};
		add(currentSubgraph, nautylink, label);
	} // end method add(2)


	/* Implement the add function of subgraph enumeration result*/
	virtual void add(Subgraph& currentSubgraph, NautyLink& nautylink, const std::string& label)
	{
		//put_time_stamp(std::cerr) << " [Thread: " << std::this_thread::get_id() << "]: " << "In SubgraphCount::add(3), aquiring lock ..." << std::endl;
		//{Logger()  << "[Thread: " << std::this_thread::get_id() << "]: " << "In SubgraphCount::add(3), aquiring lock ..." << std::endl;}
		std::lock_guard<std::mutex> guard(m_mtx_label_frq_map);
		//put_time_stamp(std::cerr) << " [Thread: " << std::this_thread::get_id() << "]: " << "In SubgraphCount::add(3), aquired lock" << std::endl;
		//{Logger()  << "[Thread: " << std::this_thread::get_id() << "]: " << "In SubgraphCount::add(3), aquired lock" << std::endl;}

		if (1 == labelFreqMap.count(label))
		{
			labelFreqMap[label] += 1;
		} // end if
		else
		{
			labelFreqMap[label] = 1;
		} // end else
		//put_time_stamp(std::cerr) << " [Thread: " << std::this_thread::get_id() << "]: " << "In SubgraphCount::add(3), releasing lock ..." << std::endl;
		//{Logger() << "[Thread: " << std::this_thread::get_id() << "]: " << "In SubgraphCount::add(3), releasing lock ..." << std::endl;}
	} // end method add(3)


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
	std::mutex m_mtx_label_frq_map;
};

#endif /* SUBGRAPHCOUNT_H */
