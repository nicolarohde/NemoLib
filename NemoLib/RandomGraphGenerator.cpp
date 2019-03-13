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

#include "RandomGraphGenerator.h"	// class header
#include "Utility.hpp"				// RNG_provider, get_random_in_range
#include <random>					// shuffle
#include <numeric>					// reduce

using std::shuffle;
using std::vector;


Graph RandomGraphGenerator::generate(Graph& inputGraph)
{
	vector<int> degreeSeq = std::move(getDegreeSequenceVector(inputGraph));
	vector<int> vertexList;
	Graph randomGraph(inputGraph.isDirected());

	// reserve memory for all vertices
	vertexList.reserve(std::reduce(degreeSeq.begin(), degreeSeq.end(), 0));

	// generate randomized list of vertices
	// the vertexList is a set where each node is represented by a number
	// of elements equal to that vertex's degree
	for (int vertex = 0; vertex < inputGraph.getSize(); vertex++)
	{
		randomGraph.addVertex();
		for (int degree = 0; degree < degreeSeq[vertex]; degree++)
		{
			vertexList.push_back(vertex);
		}
	}

	shuffle(vertexList.begin(), vertexList.end(), RNG_provider());

	// create edges
	while (!vertexList.empty())
	{
		int u = get_random_in_range<int>(0, vertexList.size() - 1);
		int v = get_random_in_range<int>(0, vertexList.size() - 1);

		for(auto i = 0; u == v; i++)
		{
			if (i > 0)
			{
				std::cerr << "looped multiple times in randomgraph generator!" << std::endl;
			}

			// this avoids looping for a long time when only a few values
			// the first case will not work if u = v = vertexList.size() - 1
			// the second case will not work if u = v = 0
			// we force these two cases into the other case
			if((get_random_in_range<int>(0, 1) % 2 == 0 && u < (vertexList.size() - 1)) || v == 0)
				v = get_random_in_range<int>(u + 1, vertexList.size() - 1);
			else
				u = get_random_in_range<int>(0, v - 1);
		}

		if (u > v)
		{
			std::swap(u, v);
		}

		int edgeVertexV = vertexList[v];
		int edgeVertexU = vertexList[u];

		vertexList.erase(vertexList.begin() + v);
		vertexList.erase(vertexList.begin() + u);

		if (edgeVertexV == edgeVertexU) continue; // avoid self-edge

		randomGraph.addEdge(edgeVertexU, edgeVertexV);
	}

	return randomGraph;
}


Graph RandomGraphGenerator::generate(Graph& inputGraph, vector <int> probs)
{
	vector<int> degreeSeq = std::move(getDegreeSequenceVector(inputGraph));
	vector <int> vertexList;
	Graph randomGraph(inputGraph.isDirected());

	vertexList.reserve(std::reduce(degreeSeq.begin(), degreeSeq.end(), 0));

	// generate randomized list of vertices
	// the vertexList is a set where each node is represented by a number
	// of elements equal to that vertex's degree
	for (int vertex = 0; vertex < inputGraph.getSize(); vertex++)
	{
		randomGraph.addVertex();
		for (int degree = 0; degree < degreeSeq[vertex]; degree++)
		{
			vertexList.push_back(vertex);
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
vector<int> RandomGraphGenerator::getDegreeSequenceVector(Graph& inputGraph) 
{
	vector <int> degreeSequenceVector(inputGraph.getSize(), 0);

	for (int currentVertex = 0; currentVertex < inputGraph.getSize(); ++currentVertex) 
	{
		degreeSequenceVector[currentVertex] = inputGraph.getAdjacencyList(currentVertex).size();
	}

	return degreeSequenceVector;
}
