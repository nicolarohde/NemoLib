/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 * File:   Graph.cpp
 * Author: Wooyoung
 *
 * Created on October 12, 2017, 1:51 PM
 */

#include "Graph.hpp"		// class header
#include <sstream>		// stringstream
#include <algorithm>	// shuffle
#include <fstream>		// ifstream
#include "Utility.hpp"	// RNG_provider

using std::string;
using std::unordered_map;


/**
 * Add a vertex to this Graph.
 * @return the ID number assigned to the new vertex
 */
std::size_t Graph::addVertex(string nodeName) 
{
	if (name2Index.count(nodeName) > 0)
	{
		std::cerr << "Same node already exists." << std::endl;
		return -1;
	} // end if

	name2Index[nodeName] = (vertex)adjacencyLists.size();

	adjacencyLists.push_back(std::unordered_set<vertex>());

	return adjacencyLists.size() - 1;
}


/**
	* Add an edge between two existing vertices on this graph.
	* @param vertexA One of the vertices between which to add an edge.
	* @param vertexB The other vertex.
	* @return true if both vertexA and vertexB exist in this Graph; false
	* otherwise.
 */
bool Graph::addEdge(vertex vertexA, vertex vertexB) 
{
	if (vertexA > adjacencyLists.size() - 1 || vertexB > adjacencyLists.size() - 1) 
	{
		return false;
	} // end if
	else
	{ // add edges for both direction
		adjacencyLists[vertexA].insert(vertexB);
		adjacencyLists[vertexB].insert(vertexA);

		edge e = 0;

		auto a_less_b = (vertexA < vertexB);
		e = a_less_b ? new_edge(vertexA, vertexB) : new_edge(vertexB, vertexA);
		edges[e] = directed ? (a_less_b ? DIR_U_T_V : DIR_V_T_U) : UNDIR_U_V;

		return true;
	} // end else
} // end method addEdge


// parses a data file into an adjacency list representing the graph
void Graph::parse(string filename) 
{
	std::vector<string> lines;
	string currentline, from, to;
	std::stringstream linesplits;
	std::ifstream in(filename, std::ios::in);
	
	// allocate enough memory for all edges
	lines.reserve(std::count(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>(), '\n'));
	in.seekg(std::ios::beg);

	if (!in.is_open() || in.bad()) 
	{
		throw std::invalid_argument("Cannot find an input file");
	} // end if
	
	// read in the entire graph
	while (in) 
	{
		getline(in, currentline);
		if (currentline.size() > 0) 
		{
			lines.push_back(currentline);
		}
	}

	in.close();

	// randomly parse input to avoid data bias
	std::shuffle(lines.begin(), lines.end(), RNG_provider());

	for (auto& line : lines) 
	{
		vertex fromIndex, toIndex;
		edge e = 0;

		linesplits << line;
		linesplits >> from >> to;
		linesplits.clear();
		linesplits.str("");

		fromIndex = getOrCreateIndex(from, name2Index);
		toIndex = getOrCreateIndex(to, name2Index);

		// don't add self edges
		if (fromIndex != toIndex)
		{
			adjacencyLists[fromIndex].insert(toIndex);
			adjacencyLists[toIndex].insert(fromIndex);

			auto a_less_b = (fromIndex < toIndex);
			e = a_less_b ? new_edge(fromIndex, toIndex) : new_edge(toIndex, fromIndex);
			edges[e] = directed ? (a_less_b ? DIR_U_T_V : DIR_V_T_U) : UNDIR_U_V;
		} // end if
	} // end for
} // end method parse


// get index of a node given the node's name
// create an entry if it does not exist
vertex Graph::getOrCreateIndex(string nodeName, unordered_map<string, vertex>& nameToIndex) 
{
	if (nameToIndex.count(nodeName) == 0) 
	{ // if the key does not exists      
		nameToIndex[nodeName] = static_cast<vertex>(adjacencyLists.size());
		adjacencyLists.push_back(std::unordered_set<vertex>());
	}

	return nameToIndex[nodeName];
}


std::ostream& operator<< (std::ostream& out, const Graph& graph)
{
	out << "Vertices: ";
	for (auto it : graph.name2Index)
	{
		out << "< " << it.first << " : " << it.second << "> ";
	}

	out << std::endl << "Edges: " << std::endl;
	for (auto ed : graph.edges)
	{
		edge code = ed.first;
		edgetype type = ed.second;
		vertex u = edge_get_u(code);
		vertex v = edge_get_v(code);
		out << "<";

		switch (type)
		{
		case DIR_U_T_V:
			out << u << "->" << v;
			break;
		case DIR_V_T_U:
			out << v << "->" << u;
			break;
		case UNDIR_U_V:
			out << u << "--" << v;
			break;
		default:
			throw std::invalid_argument("Invalid edge type!");
		}
		out << "> ";
	}

	out << std::endl << (graph.directed ? "Directed" : "Undirected") << " Graph, |V| = " << graph.adjacencyLists.size() << ": |E| = " << graph.edges.size() << std::endl;

	return out;
}
