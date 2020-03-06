/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

 /*
  * File:   Subgraph.h
  * Author: Wooyoung
  *
  * Created on October 23, 2017, 2:34 PM
  * Note that this Subgraph class is different from Java version, where the adjacentmatrix is necessary
  * It will inherit most of the functions from Graph
  *
  */
#pragma once
#ifndef SUBGRAPH_H
#define SUBGRAPH_H


#include "Config.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "graph64.hpp"

// using-statements pollute the files that include this header
// so they were removed to prevent unintended side-effects for users


class Subgraph
{
public:
	friend std::ostream& operator<< (std::ostream&, const Subgraph&);

	// Constructor initializes class members
	Subgraph(std::size_t _order) : order(_order), current(0), nodes(_order, 0) {}


	// compiler generated destructor will work for this class
	virtual ~Subgraph() = default;


	// Regular copy mechanism
	Subgraph(const Subgraph& OTHER)
	{
		*this = OTHER;
	} // end Copy Constructor


	// Efficient copy mechanism that uses move semantics
	// noexcept qualifier as STL uses std::move_if_noexcept
	Subgraph(Subgraph&& other) noexcept
	{
		*this = std::move(other);
	} // end Move Constructor


	// Regular copy mechanism
	Subgraph& operator=(const Subgraph& OTHER)
	{
		order = OTHER.order;
		current = OTHER.current;
		nodes = std::vector<vertex>(OTHER.nodes);

		return *this;
	} // end Copy Assignment


	// Efficient copy mechanism that uses move semantics
	// noexcept qualifier as STL only uses std::move_if_noexcept
	Subgraph& operator=(Subgraph&& other) noexcept
	{
		order = other.order;
		current = other.current;
		nodes = std::move(other.nodes);

		return *this;
	} // end Move Assignment


	// allows use of range-based functions with the subgraph class
	inline auto begin(void)
	{
		return nodes.begin();
	} // end method begin


	// allows use of range-based functions with the subgraph class
	inline auto end(void)
	{
		return nodes.end();
	} // end method end

	// adds next element to nodes
	inline void add(vertex v)
	{
		if (isComplete())
		{
			std::cout << "The subgraph is full:: Cannot add more" << std::endl;
			return;
		} // end if

		// nodes' memory is already allocated to avoid expensive resizing
		// current keeps track of size and next element to insert
		nodes[current] = v;
		current++;
	} // end method add


	// to avoid signed -> unsigned truncation errors,
	// we cannot return a sentinel value in this function
	inline vertex get(std::size_t n) const
	{
		return nodes[n];
	} // end method get


	inline std::vector<vertex>& getNodes()
	{
		return nodes;
	} // end method getNodes


	// to avoid signed -> unsigned truncation errors,
	// we cannot return a sentinel value in this function
	// get the first vertex id added to this subgraph
	inline vertex root(void) const
	{
		return nodes[0];
	} // end method root


	// whether or not more nodes can be added
	inline bool isComplete(void) const
	{
		return (current == order);
	} // end method isComplete

	inline std::size_t getOrder(void) const
	{
		return order;
	} // end method getOrder

	inline std::size_t getSize(void) const
	{
		return current;
	} // end method getSize

	inline bool contains(vertex v) const //Check whether a vertex exists in this Subgraph
	{
		return std::find(std::begin(nodes), std::end(nodes), v) != std::end(nodes);
	} // end method contains

private:
	std::vector<vertex> nodes; // get the list of vertex lists
	std::size_t order; // the intended size
	std::size_t current;
};


inline std::ostream& operator<<(std::ostream& out, const Subgraph& sgraph)
{
	if (sgraph.nodes.size() < 1)
	{
		out << "empty";
	}
	else
	{
		out << "[";

		for (std::size_t i = 0; i < sgraph.nodes.size() - 1; i++)
		{
			out << std::to_string(sgraph.nodes[i]) << ",";
		}

		out << std::to_string(sgraph.nodes[sgraph.nodes.size() - 1]) << "]";
	}

	return out;
}

#endif
