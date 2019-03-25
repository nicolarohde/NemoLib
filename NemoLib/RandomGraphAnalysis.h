/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   RandomGraphAnalysis.h
 * Author: Wooyoung
 *
 * Created on November 3, 2017, 1:29 PM
 */

/**
 * RandomGraphAnalysis is a facade class that generates and enumerates using
 * RAND-ESU a set of random network graphs based on the degree sequence vector
 * of a specified graph. The output of the analyze() method can be used by a
 * StatisticalAnalysis object to determine whether a target graph contains any
 * network motifs.
 */

#ifndef RANDOMGRAPHANALYSIS_H
#define RANDOMGRAPHANALYSIS_H

#include "Config.hpp"
#include "graph64.hpp"
#include <unordered_map>
#include <vector>

// forward declare type for references
class Graph;

class RandomGraphAnalysis
{
public:
    // not allowed 
    RandomGraphAnalysis(void) = delete;
    static std::unordered_map<graph64, std::vector<double>> analyze(Graph&, int, int, const std::vector<double>&);
};

#endif /* RANDOMGRAPHANALYSIS_H */
