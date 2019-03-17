/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

 /*
  * File:   Graph.h
  * Author: Wooyoung
  *
  * Created on October 12, 2017, 1:51 PM
  */

#ifndef GRAPH_H
#define GRAPH_H
#include "Config.hpp"
#include <iostream>			// ostream
#include <stdexcept>		// invalid_argument
#include <vector>			// vector
#include <unordered_set>	// unordered_set 
#include <unordered_map>	// unordered_map
#include "graph64.hpp"		// vertex, edgetype


class Graph 
{
public:
	Graph(bool dir = false) noexcept : directed(dir) {}
	

	Graph(const std::vector<int>& _EDGES, const std::size_t _N, const bool _DIR = false) : directed(_DIR)
	{
		addVertices(_N);
		addEdges(_EDGES);
	}


	Graph(std::string filename, bool dir = false) : directed(dir)
	{ 
		parse(filename); 
	};


	Graph(const Graph& OTHER) 
	{
		*this = OTHER; 
	}


	Graph(Graph&& other) noexcept 
	{ 
		*this = other;
	}


	Graph& operator=(const Graph& OTHER)
	{
		directed = OTHER.directed;
		adjacencyLists = std::vector<std::unordered_set<vertex>>(OTHER.adjacencyLists);
		name2Index = std::unordered_map<std::string, vertex>(OTHER.name2Index);
		edges = std::unordered_map<edge, edgetype>(OTHER.edges);
		return *this;
	}


	Graph& operator=(Graph&& other) noexcept
	{
		directed = other.directed;
		adjacencyLists = std::move(other.adjacencyLists);
		name2Index = std::move(other.name2Index);
		edges = std::move(other.edges);
		return *this;
	}


	virtual ~Graph() = default;


	inline bool isDirected() noexcept
	{
		return directed;
	} // end method 


	inline auto& getAdjacencyList(vertex index)
	{
		return adjacencyLists.at(index);
	} // end method 


	inline auto& getNametoIndex() noexcept
	{
		return name2Index;
	} // end method 


	inline auto getIndextoName() noexcept // will be useful if need to match the index to name
	{
		std::unordered_map<vertex, std::string> Index2name;
		for (auto v : name2Index)
		{
			Index2name[v.second] = v.first;
		}

		return Index2name;
	} // end method 


	inline auto& getEdges() // added by following the original ESU program
	{
		return edges;
	} // end method 


	inline std::size_t addVertex() noexcept
	{
		adjacencyLists.push_back(std::unordered_set<vertex>());
		return adjacencyLists.size() - 1;
	}


	inline void addVertices(std::size_t n)
	{
		adjacencyLists.reserve(adjacencyLists.size() + n);
		for (auto i = 0; i < n; i++)
		{
			adjacencyLists.push_back(std::unordered_set<vertex>());
		}
	}

	inline void addEdges(const std::vector<int> EDGES)
	{
		edges.reserve(edges.size() + (EDGES.size() / 2));
		for (auto i = 0; i < EDGES.size(); i += 2)
		{
			addEdge(EDGES[i], EDGES[i + 1]);
		}
	}


	inline std::size_t getSize() noexcept
	{
		return adjacencyLists.size();
	} // end method 


	// might not need --> will delete later
	class Edge 
	{
	private:
		vertex nodeA;
		vertex nodeB;

	public:
		Edge() = default;
		Edge(vertex _nodeA, vertex _nodeB) noexcept : nodeA(_nodeA), nodeB(_nodeB) {}
		Edge(const Edge&) = default;
		Edge(Edge&&) = default;
		Edge& operator=(const Edge&) = default;
		Edge& operator=(Edge&&) = default;

		inline bool equals(Edge& edge, bool dir) noexcept
		{
			return ((edge.nodeA == this->nodeA) && (edge.nodeB == this->nodeB)) || (dir ? false : ((edge.nodeA == this->nodeB) && (edge.nodeB == this->nodeA)));
		}

		inline friend std::ostream& operator<< (std::ostream& out, const Graph::Edge& _edge)
		{
			out << "[" << _edge.nodeA << "," << _edge.nodeB << "]" << std::endl;
			return out;
		}
	};

	std::size_t addVertex(std::string);

	bool addEdge(vertex, vertex);

	friend std::ostream& operator<< (std::ostream&, const Graph&);

private:
	std::vector<std::unordered_set<vertex>> adjacencyLists;
	std::unordered_map<std::string, vertex> name2Index;
	std::unordered_map<edge, edgetype> edges;

	bool directed;

	void parse(std::string);
	vertex getOrCreateIndex(std::string, std::unordered_map<std::string, vertex>&);
};

#endif /* GRAPH_H */

