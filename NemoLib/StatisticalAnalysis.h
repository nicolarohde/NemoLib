/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

 /*
  * File:   StatisticalAnalysis.h
  * Author: Wooyoung
  *
  * Created on November 3, 2017, 2:32 PM
  */
#pragma once
#ifndef STATISTICALANALYSIS_H
#define STATISTICALANALYSIS_H

#include "Config.hpp"
#include "graph64.hpp"
#include <unordered_map>	// unordered_map
#include <vector>			// vector
#include <numeric>			// reduce, accumulate
#include <cmath>			// pow

#if _C17_EXECUTION_AVAILABLE
#include <execution>
#endif

class StatisticalAnalysis
{

public:
	StatisticalAnalysis() = delete; // not allowed
	StatisticalAnalysis(std::unordered_map<graph64, double>& _targetGraphRelFreqsMap, std::unordered_map<graph64, std::vector<double>>& _randomGraphRelFreqsMap, int _randGraphCount)
		: targetGraphRelFreqs(_targetGraphRelFreqsMap), randomGraphRelFreqs(_randomGraphRelFreqsMap), randGraphCount(_randGraphCount){}

	virtual ~StatisticalAnalysis() = default;

	std::unordered_map<graph64, double> getZScores();
	std::unordered_map<graph64, double> getPValues();
	double getPValue(graph64);

	friend std::ostream& operator<< (std::ostream&, StatisticalAnalysis&);

	inline double getZScore(graph64 label)
	{
		double randMean = calcRandMean(label);
		return getZScore(label, randMean, calcRandStdDev(label, randMean));
	}


	inline double getZScore(graph64 label, double mean, double stdDev)
	{
		double targetGraphFreq = targetGraphRelFreqs.count(label) == 0 ? 0.0 : targetGraphRelFreqs[label];
		return (stdDev == 0 ? 0.0 : (targetGraphFreq - mean) / stdDev);
	}

private:
	std::unordered_map<graph64, double> targetGraphRelFreqs;
	std::unordered_map<graph64, std::vector<double>> randomGraphRelFreqs;
	int randGraphCount;


	inline double calcRandStdDev(graph64 label, double randMean)
	{
		#if _C17_EXECUTION_AVAILABLE
		double variance = std::reduce(std::execution::par_unseq, randomGraphRelFreqs[label].begin(), randomGraphRelFreqs[label].end(), 0.0,
			[&](auto prev, auto cur) { return prev + std::pow((cur - randMean), 2); });
		#else
		double variance = std::accumulate(randomGraphRelFreqs[label].begin(), randomGraphRelFreqs[label].end(), 0.0,
			[&](auto prev, auto cur) { return prev + std::pow((cur - randMean), 2); });
		#endif

		return std::sqrt(variance / (randGraphCount - 1));
	}


	inline double calcRandMean(graph64 label)
	{
		auto* relFreqs = &randomGraphRelFreqs[label];

		// reduce uses reduction tree O(logn)
		#if _C17_EXECUTION_AVAILABLE
		double total = std::reduce(std::execution::par_unseq, relFreqs->begin(), relFreqs->end(), 0.0);
		#else
		double total = std::accumulate(relFreqs->begin(), relFreqs->end(), 0.0);
		#endif

		return (total / static_cast<double>(relFreqs->size()));
	}
};

#endif 

