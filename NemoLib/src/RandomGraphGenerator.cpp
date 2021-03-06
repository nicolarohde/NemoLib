/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

 /*
  * File:   RandomGraphGenerator.cpp
  * Author: Wooyoung
  *
  * Created on November 2, 2017, 1:23 PM
  */

#include "Config.hpp"
#include "RandomGraphGenerator.hpp"	// class header
#include "Utility.hpp"				// RNG_provider, get_random_in_range, get_vector_sum
#include <algorithm>				// shuffle
#include "Logger.hpp"

#include "loguru.hpp"

using std::shuffle;
using std::vector;


Graph RandomGraphGenerator::generate(const Graph& inputGraph)
{
    DLOG_F(DEBUG_LEVEL, "In RandomGraphGenerator::generate ... ");

	vector<int> degreeSeq = std::move(getDegreeSequenceVector(inputGraph));
	vector<vertex> vertexList;
	Graph randomGraph(inputGraph.isDirected());

    DLOG_F(DEBUG_LEVEL, "Setup complete");

	// reserve memory for all vertices
	vertexList.reserve(get_vector_sum(degreeSeq.begin(), degreeSeq.end()));

    DLOG_F(DEBUG_LEVEL, "Reserved memory for vertices");

	// generate randomized list of vertices
	// the vertexList is a set where each node is represented by a number
	// of elements equal to that vertex's degree
	for (vertex vert = 0; vert < inputGraph.getSize(); vert++)
	{
		randomGraph.addVertex();
		for (int degree = 0; degree < degreeSeq[vert]; degree++)
		{
			vertexList.push_back(vert);
		} // end for degree
	} // end for vertex

    DLOG_F(DEBUG_LEVEL, "Graph vertexList creation complete");

	shuffle(vertexList.begin(), vertexList.end(), RNG_provider());

    DLOG_F(DEBUG_LEVEL, "Graph vertexList shuffle complete");

	std::size_t count{1};

	// create edges
	while (!vertexList.empty())
	{
		count ++;
		auto n_vertices = vertexList.size();

		if (0 == count % 10000) 
		{ 
            LOG_F(INFO, "On %zu loop of randomization (at least %zu remaining).", count, n_vertices / 2);
		}

		std::size_t u = get_random_in_range<std::size_t>(0ULL, n_vertices - 1ULL);
		std::size_t v = get_random_in_range<std::size_t>(0ULL, n_vertices - 1ULL);

		// this avoids looping for a long time when only a few values
		// the first case will not work if u = v = vertexList.size() - 1
		// the second case will not work if u = v = 0
		// we force these two cases into the other case
		if (u == v)
		{
            DLOG_F(DEBUG_LEVEL, "u == v case");

			if ((get_random_in_range<std::size_t>(0ULL, 1ULL) % 2ULL == 0ULL && u < (n_vertices - 1ULL)) || v == 0ULL)
			{
				v = get_random_in_range<std::size_t>(u + 1ULL, n_vertices - 1ULL);
			} // end if
			else
			{
				u = get_random_in_range<std::size_t>(0ULL, v - 1ULL);
			} // end if
		} // end else

		if (u > v)
		{
			std::swap(u, v);
		} // end if

		vertex edgeVertexV = vertexList[v];
		vertex edgeVertexU = vertexList[u];

		vertexList.erase(vertexList.begin() + v);
		vertexList.erase(vertexList.begin() + u);

		if (edgeVertexV == edgeVertexU) continue; // avoid self-edge

		randomGraph.addEdge(edgeVertexU, edgeVertexV);
	} // end while

    DLOG_F(DEBUG_LEVEL, "Random graph creation done, iterations of loop: %zu", count);

	return randomGraph;
} // end method generate


Graph RandomGraphGenerator::generate(const Graph& inputGraph, const vector <int>& probs)
{
	vector<int> degreeSeq = std::move(getDegreeSequenceVector(inputGraph));
	vector<int> vertexList;
	Graph randomGraph(inputGraph.isDirected());

	vertexList.reserve(get_vector_sum(degreeSeq.begin(), degreeSeq.end(), 0, [&](auto x, auto y){return x+y;}));

	// generate randomized list of vertices
	// the vertexList is a set where each node is represented by a number
	// of elements equal to that vertex's degree
	for (int vert = 0; static_cast<std::size_t>(vert) < inputGraph.getSize(); vert++)
	{
		randomGraph.addVertex();
		for (int degree = 0; degree < degreeSeq[vert]; degree++)
		{
			vertexList.push_back(vert);
		}
	}

	shuffle(vertexList.begin(), vertexList.end(), RNG_provider());

	// create edges
	auto it = probs.begin();

	while (!vertexList.empty())
	{
		auto u = it;
		std::advance(it, 1);

		// make sure v != u

		if (it == probs.end())
		{
			break;
		}

		auto v = it;
		std::advance(it, 1);

		while (*v == *u)
		{
			if (it == probs.end())
			{
				break;
			}
			v = it;
			std::advance(it, 1);
		}

		if (*u > *v)
		{
			std::swap(u, v);
		}

		int edgeVertexV = vertexList[*v];
		int edgeVertexU = vertexList[*u];

		if (edgeVertexV == edgeVertexU) continue; // avoid self-edge

		vertexList.erase(v);
		vertexList.erase(u);

		randomGraph.addEdge(edgeVertexU, edgeVertexV);
	}
	return randomGraph;
}


/**
 * Generates a degree sequence vector for a given Graph
 * @param inputGraph the Graph from which to derive the degree sequence
 * vector
 * @return a List representing the degree sequence vector
 */
vector<int> RandomGraphGenerator::getDegreeSequenceVector(const Graph& inputGraph)
{
	vector <int> degreeSequenceVector(inputGraph.getSize(), 0);

	for (int currentVertex = 0; static_cast<std::size_t>(currentVertex) < inputGraph.getSize(); currentVertex++)
	{
		degreeSequenceVector[currentVertex] = static_cast<int>(inputGraph.getAdjacencyList(currentVertex).size());
	}

	return degreeSequenceVector;
}
