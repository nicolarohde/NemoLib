/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   RandomGraphGenerator.h
 * Author: Wooyoung
 *
 * Created on November 2, 2017, 1:22 PM
 */



#ifndef RANDOMGRAPHGENERATOR_H
#define RANDOMGRAPHGENERATOR_H

#include "Config.hpp"
#include "Graph.hpp"

/**
  * Generates random graphs from an input graph based on the degree sequence of 
  * the original graph.
  */

class RandomGraphGenerator 
{
public:
    static Graph generate(const Graph&);
    static Graph generate(const Graph&, const std::vector<int>&);

private:
    static std::vector<int> getDegreeSequenceVector(const Graph&);
};

#endif /* RANDOMGRAPHGENERATOR_H */

