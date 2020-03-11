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
#include "Graph.hpp"					 // Graph
#include "Subgraph.hpp"					 // Subgraph
#include "NautyLink.hpp"				 // NautyLink
#include "SubgraphEnumerationResult.hpp" // SubgraphEnumerationResult
#include "Utility.hpp"					 // get_random_in_range
#include <vector>						 // vector
#include <cassert>						 // assert
#include <algorithm>		             // copy_if
#include <numeric>			             // iota
#include <unordered_set>	             // unordered_set
#include "Logger.hpp"

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
	template <typename T>
    static void enumerate(Graph& graph, T* subgraphs, int subgraphsize, const std::vector<double>& probs, const std::string& labelg_path)
	{
		std::size_t numVerticesToSelect = probs[0] == 1.0 ? graph.getSize() : static_cast<std::size_t>(round(probs[0] * graph.getSize()));

		//{Logger()  << "[Thread: " << std::this_thread::get_id() << "]: " << "In RandESU::numerate(top)" << std::endl;}

		// maintain list of nodes selected so far
		std::vector<vertex> selectedVertices(numVerticesToSelect);

		if (probs[0] == 1.0) // select all nodes
		{
			std::iota(selectedVertices.begin(), selectedVertices.end(), 0);
		} // end if
		else 
		{
			std::unordered_set<vertex> seen;

			for (auto& current : selectedVertices) 
			{
				vertex nodeSelected = get_random_in_range<vertex>(0, static_cast<vertex>(graph.getSize() - 1)); // get the node id

				while (seen.count(nodeSelected) > 0)
				{
					nodeSelected = get_random_in_range<vertex>(0, static_cast<vertex>(graph.getSize() - 1));
				} // end while

				seen.insert(nodeSelected);

				current = nodeSelected;
			} // end for current
		} // end else

		//{Logger()  << "[Thread: " << std::this_thread::get_id() << "]: " << "Creating nautylink ..." << std::endl;}

		NautyLink nautylink(labelg_path, subgraphsize, graph.getEdges(), graph.isDirected());

		//{Logger()  << "[Thread: " << std::this_thread::get_id() << "]: " << "Enumerating ..." << std::endl;}

		for (auto v : selectedVertices) 
		{
			enumerate<T>(graph, subgraphs, subgraphsize, probs, v, nautylink);
		} // end for v
	} // end method enumerate(4)


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
	template <typename T>
    static void enumerate(Graph& graph, T* subgraphs, int subgraphsize, const std::vector<double>& probs, vertex vertexV, NautyLink& nautylink)
	{
		//{Logger()  << "[Thread: " << std::this_thread::get_id() << "]: " << "In RandESU::numerate(bottom)" << std::endl;}

		// create a subgraph with given subgraphsize
		Subgraph subgraph(subgraphsize);

		// create an extends
		std::unordered_set<vertex> adjacencyList = graph.getAdjacencyList(vertexV);
		std::vector<vertex> extends;
		extends.reserve(adjacencyList.size());

		std::copy_if(adjacencyList.begin(), adjacencyList.end(), std::back_inserter(extends), 
			[&](auto& x) 
			{ 
				return x > vertexV; 
			} // end lambda
		); // end copy_if

		subgraph.add(vertexV); // add to the subgraph, the vertex and its corresponding adjacencylist

		// randomly decide whether to extend
		if (shouldExtend(probs[1])) 
		{
			//std::cerr << "Calling extend ..." << std::endl;
			extend<T>(graph, subgraph, std::move(extends), probs, subgraphs, nautylink);
		} // end if
	} // end method enumerate(6)



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
	template <typename T>
    static void extend(Graph& graph, Subgraph& subgraph, std::vector<vertex> extension, const std::vector<double>& probs, T* subgraphs, NautyLink& nautylink)
	{
		// optimize by not creating next extension if subgraph is
		// 1 node away from completion
		if (subgraph.getSize() == subgraph.getOrder() - 1) 
		{
			for(auto& element : extension)
			{
				// check the last value in prob list
				if (shouldExtend(probs.at(probs.size() - 1))) 
				{
					Subgraph subgraphUnion(subgraph);
					subgraphUnion.add(element);
					//std::cerr << "Calling subgraph->add ..." << std::endl;
					subgraphs->add(subgraphUnion, nautylink);
				} // end if
			} // end for element
		} // end if
		else
		{
			//std::cerr << "In else of extend ..." << std::endl;

			vertex v = subgraph.root();

			// iterating over a vector and erasing items invalidates the iterator
			// instead process first element until vector is empty
			while (!extension.empty())
			{
				vertex w = extension[0];
				extension.erase(extension.begin());

				std::vector<vertex> nextExtension(extension);
				std::unordered_set<vertex> adjW = graph.getAdjacencyList(w);

				for(auto& u : adjW)
				{
					if (u > v && isExclusive(graph, u, subgraph))
					{
						nextExtension.push_back(u);
					} // end if
				} // end for u

				// construct a union of w and the existing subgraph
				Subgraph subgraphUnion(subgraph);
				subgraphUnion.add(w);

				// randomly choose whether or not to extend to the next level
				// based on the probability vector provided.
				if (shouldExtend(probs.at(subgraphUnion.getSize() - 1)))
				{
					extend<T>(graph, subgraphUnion, nextExtension, probs, subgraphs, nautylink);
				} // end if
			} // end while
		} // end else
	} // end method extend
};

#endif /* RANDESU_H */
