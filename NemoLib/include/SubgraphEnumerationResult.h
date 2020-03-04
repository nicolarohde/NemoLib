/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   SubgraphEnumerationREsult.h
 * Author: Wooyoung
 *
 * Created on October 18, 2017, 4:34 PM
 */

#ifndef SUBGRAPHENUMERATIONRESULT_H
#define SUBGRAPHENUMERATIONRESULT_H
/*This class is an interface class*/

#include <unordered_map>

#include "Config.hpp"
#include "graph64.hpp"

// forward declare types for references below
class Subgraph;
class NautyLink;

class SubgraphEnumerationResult
{
public:
    /**
	 * Adds a Subgraph to this SubgraphEnumerationResult
	 * @param currentSubgraph a Subgraph to add to this
	 * SubgraphEnumerationResult
	 */
    virtual void add(Subgraph&, NautyLink&) = 0;

    /**
	 * Calculates and returns a map of relative frequencies or concentrations
	 * of labels in this subgraph.
	 * @return The map of relative frequencies
	 */
    virtual std::unordered_map <graph64, double> getRelativeFrequencies(void) = 0;
};

#endif /* SUBGRAPHENUMERATIONRESULT_H */
