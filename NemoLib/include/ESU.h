/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ESU.h
 * Author: Wooyoung
 *
 * Created on October 25, 2017, 1:06 PM
 */

#ifndef ESU_H
#define ESU_H
#include "Config.hpp"
#include "RandESU.h"


/**
 * ESU is a static class used for executing the Enumerate Subgraphs algorithm
 * on a network graph.
 */
class ESU 
{
public:
   /**
	 * Enumerates Subgraphs using the ESU algorithm. Requires user to specify
	 * return type(s) and provide the accompanying data structures.
	 *
	 * @param graph the graph on which to execute ESU
	 * @param subgraphs the SubgraphEnumerationResult into which to enumerated
	 *                  Subgraphs will be stored.
	 * @param subgraphSize the size of the target Subgraphs
	 */
	static void enumerate(Graph& graph, SubgraphEnumerationResult* subgraphs, int subgraphSize)
	{
		NautyLink nautylink(subgraphSize, graph.getEdges(), graph.isDirected());

		for (std::size_t i = 0; i < graph.getSize(); i++)
		{
			enumerate(graph, subgraphs, subgraphSize, i, nautylink);
		} // end for i
	} // end method enumerate

   /**
	 * Enumerates Subgraphs for one branch of the ESU tree starting at the
	 * given node. Allows for more control over the order the order of 
	 * execution, but does not perform a full enumeration.
	 *
	 * @param graph the graph on which to execute ESU
	 * @param subgraphs the data structure to which results are written
	 * @param subgraphSize the target subgraph size to enumerate
	 * @param vertex the graph vertex at which to execute
     * @param nautylink after initialize from the previous
	 */
    inline static void enumerate(Graph& graph, SubgraphEnumerationResult* subgraphs, int subgraphSize, vertex vertexV, NautyLink& nautylink)
	{
		std::vector<double> probs(subgraphSize, 1.0);
		RandESU::enumerate(graph, subgraphs, subgraphSize, probs, vertexV, nautylink);
	} // end method enumerate
};

#endif /* ESU_H */

