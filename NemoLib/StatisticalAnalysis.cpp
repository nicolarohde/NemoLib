/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

 /*
  * File:   StatisticalAnalysis.cpp
  * Author: Wooyoung
  *
  * Created on November 3, 2017, 2:32 PM
  */

#include "StatisticalAnalysis.h"	// class header
#include <numeric>					// reduce
#include <iostream>					// ostream
#include <execution>				// execution::par_unseq

using std::unordered_map;
using std::vector;
using std::ostream;

/**
 * Subsystem to store relative frequencies for each subgraph pattern / label and
 * calculate the p-values and z-scores for those labels.
 */


unordered_map<graph64, double> StatisticalAnalysis::getZScores()
{
	unordered_map<graph64, double> zScores(randomGraphRelFreqs.size());

	for (auto& p : randomGraphRelFreqs)
	{
		graph64 label = p.first;
		double randMean = calcRandMean(label);
		double randStdDev = calcRandStdDev(label, randMean);
		double targetGraphFreq = targetGraphRelFreqs.count(label) == 0 ? 0.0 : targetGraphRelFreqs[label];

		zScores[label] = randStdDev == 0 ? 0.0 : (targetGraphFreq - randMean) / randStdDev;
	}
	return zScores;
}


unordered_map<graph64, double> StatisticalAnalysis::getPValues(void)
{
	unordered_map<graph64, double>  pValues;
	for (auto& p : randomGraphRelFreqs)
	{
		pValues[p.first] = getPValue(p.first);
	}
	return pValues;
}


double StatisticalAnalysis::getPValue(graph64 label) 
{
	// if a label appears in the target graph that didn't show up in any
	// random graphs, clearly it's a network motif. This scenario shouldn't
	// happen for a reasonable number of random graphs
	if (randomGraphRelFreqs.count(label) == 0) 
	{
		return 0.0;
	}

	// This shouldn't happen with the current design, but in case the design
	// changes to include functionality to display all labels found in the
	// random graphs instead of just those in the target graph, this will
	// ensure those labels are not identified as network motifs
	if (targetGraphRelFreqs.count(label) == 0) 
	{
		return 1.0;
	}

	double targetFreq = targetGraphRelFreqs[label];
	std::size_t prePValue = std::count_if(randomGraphRelFreqs[label].begin(), randomGraphRelFreqs[label].end(), [&](auto& x) { return x > targetFreq; });

	return static_cast<double>(prePValue) / static_cast<double>(randomGraphRelFreqs[label].size());
}


ostream& operator<<(ostream& out, StatisticalAnalysis& rstat) 
{
	out << "Label\tRelFreq\t\tMean\t\tStDev\t\tZ-Score\t\tP-Value\n";
	out.precision(3);

	for (auto& p : rstat.randomGraphRelFreqs) 
	{
		graph64 label = p.first;
		out << label << "\t";
		if (rstat.targetGraphRelFreqs.count(label) > 0) 
		{
			out << std::fixed << rstat.targetGraphRelFreqs[label] * 100.0;
		}
		else 
		{
			out << 0.000;
		}
		out << "%\t\t";
		double mean = rstat.calcRandMean(label);
		out << mean * 100 << "%\t\t";
		double stDev = rstat.calcRandStdDev(label, mean);
		out << stDev << "\t\t";
		double zScore = rstat.getZScore(label, mean, stDev);
		out << zScore << "\t\t";
		double pValue = rstat.getPValue(label);
		out << pValue << "\n";
	}

	return out;
}

