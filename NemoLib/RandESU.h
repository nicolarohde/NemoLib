/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   RandESU.h
 * Author: Wooyoung
 *
 * Created on October 25, 2017, 10:28 AM
 */

/**
 * RandESU is a static class used for executing the RandESU on a portion of a
 * network graph.
 */

#ifndef RANDESU_H
#define RANDESU_H

#include "Config.hpp"
#include "Graph.h"						// Graph
#include "Subgraph.h"					// Subgraph
#include "NautyLink.h"					// NautyLink
#include "SubgraphEnumerationResult.h"	// SubgraphEnumerationResult
#include "Utility.hpp"					// get_random_in_range
#include <vector>						// vector
#include <cassert>						// assert

class RandESU 
{
public:
    /**
	 * Enumerates all subgraphSize Subgraphs in the input Graph using the 
	 * RAND-ESU algorithm.
	 *
	 * @param graph           the graph on which to execute RAND-ESU
	 * @param subgraphSize    the size of the target Subgraphs
	 */
    static void enumerate(Graph&, SubgraphEnumerationResult*, int, const std::vector<double>&);
    /**
	 * Enumerates all subgraphSize Subgraphs for the specified vertice's branch
	 * of an ESU execution tree using the RAND-ESU algorithm. Allows for more
	 * control over execution order compared to the enumerate method that does
	 * not include a vertex parameter.
	 * @param graph the graph on which to execute RAND-ESU
	 * @param subgraphs
	 * @param subgraphSize
	 * @param probs
	 * @param vertex
     * @param nuatylink
	 */
    static void enumerate(Graph&, SubgraphEnumerationResult*, int, const std::vector<double>&, vertex, NautyLink&);
    
    
        
private:
    
    /** determines whether or not to extend based on a given probability, given
	 as an integer.
	 precondition: 0.0 <= prob <= 1.0
    **/
    static bool shouldExtend(double prob)
	{
		assert(prob >= 0.0 && prob <= 1.0);

		return prob == 1.0 ? true : prob == 0.0 ? false : get_random_in_range(0, 100) <= (prob * 100.0);
	} // end method shouldExtend
    
    /** returns true if the node index is exclusive to the given subgraph
	 (that is, is not already in the subgraph, and is not adjacent to any of
	  the nodes in the subgraph)
     **/
    static bool isExclusive(Graph& graph, vertex node, Subgraph& subgraph)
	{
		for (auto& element : subgraph)
		{
			if (element == node || graph.getAdjacencyList(element).count(node) > 0)
			{
				return false;
			} // end if
		} // end for element

		return true;
	} // end method isExclusive
    
    /** extend the subgraphs recursively    
     **/
    static void extend(Graph&, Subgraph&, std::vector<vertex>, const std::vector<double>&, SubgraphEnumerationResult*, NautyLink&);
 

};

#endif /* RANDESU_H */

