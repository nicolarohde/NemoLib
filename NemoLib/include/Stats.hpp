#pragma once
#ifndef _STATS_ANALYSIS__H
#define _STATS_ANALYSIS__H

#include "Config.hpp"
#include "graph64.hpp"
#include "Utility.hpp"
#include <unordered_map>	// unordered_map
#include <vector>			// vector
#include <cmath>			// pow
#include <iostream>			// ostream
#include <algorithm>

namespace Statistical_Analysis
{
	// Forward declaration of namespace members:
	struct stats_data;
	std::unordered_map<std::string, double> getZScores(stats_data& data);
	std::unordered_map<std::string, double> getPValues(stats_data& data);
	double getPValue(const std::string& label, stats_data& data);
	double getZScore(const std::string& label, double mean, double stdDev, stats_data& data);
	double getZScore(const std::string& label, stats_data& data);
	double calcRandStdDev(const std::string& label, double randMean, stats_data& data);
	double calcRandMean(const std::string& label, stats_data& data);


	///<summary>
	///   Provides relevant details to calculate statistics to functions in this namespace.
	///</summary>
	///<remarks>
	///   Use of a struct reference is faster than copying large maps into the class object;
	///   this also reduces the overall memory footprint as we can reuse the objects.
	///</remarks>
	struct stats_data
	{
		stats_data(std::unordered_map<std::string, double>* _target, std::unordered_map<std::string, std::vector<double>>* _random, std::size_t _count)
			: targetGraphRelFreqs{ _target }, randomGraphRelFreqs{ _random }, randGraphCount{_count} {}

		///<summary>
		/// Relative frequencies in the target graph.
		///</summary>
		std::unordered_map<std::string, double>* targetGraphRelFreqs;

		///<summary>
		/// Relative frequencies in the randomly generated graph(s).
		///</summary>
		std::unordered_map<std::string, std::vector<double>>* randomGraphRelFreqs;

		///<summary>
		/// Number of randomly generated graphs used to find <see cref="Statistical_Analysis::stats_data::randomGraphRelFreqs"/>.
		///</summary>
		std::size_t randGraphCount;
	}; // end struct stats_data


	///<summary>
	///   Calculates the z-scores for all motif-candidates in <see cref="Statistical_Analysis::stats_data::randomGraphRelFreqs"/>.
	///</summary>
	///<param name="data"><see cref="Statistical_Analysis::stats_data"/> object with statistical data for z-score calculations.</param>
	///<returns>A map of {label : z-score} pairs.</returns>
	std::unordered_map<std::string, double> getZScores(stats_data& data)
	{
		std::unordered_map<std::string, double> zScores(data.randomGraphRelFreqs->size());

		for (auto& p : *(data.randomGraphRelFreqs))
		{
			double randMean = calcRandMean(p.first, data);
			double randStdDev = calcRandStdDev(p.first, randMean, data);
			double targetGraphFreq = data.targetGraphRelFreqs->count(p.first) == 0 ? 0.0 : (*(data.targetGraphRelFreqs))[p.first];

			zScores[p.first] = randStdDev == 0 ? 0.0 : (targetGraphFreq - randMean) / randStdDev;
		} // end for p
		return zScores;
	} // end method getZScores


	///<summary>
	///   Calculates the p-values for all motif-candidates in <see cref="Statistical_Analysis::stats_data::randomGraphRelFreqs"/>.
	///</summary>
	///<param name="data"><see cref="Statistical_Analysis::stats_data"/> object with statistical data for p-value calculations.</param>
	///<returns>A map of {label : p-value} pairs.</returns>
	std::unordered_map<std::string, double> getPValues(stats_data& data)
	{
		std::unordered_map<std::string, double>  pValues(data.randomGraphRelFreqs->size());
		for (auto& p : *(data.randomGraphRelFreqs))
		{
			pValues[p.first] = getPValue(p.first, data);
		} // end for p
		return pValues;
	} // end method getPValues


	///<summary>
	///   Calculates the p-value for the motif-candidate <paramref name="label"/>.
	///</summary>
	///<param name="label">The label for which the p-value should be calculated.</param>
	///<param name="stats_data"><see cref="Statistical_Analysis::stats_data"/> object with statistical data for p-value calculations.</param>
	///<returns>
	/// 1) 0 if the motif never occurred in the random graph(s);
	/// 2) 1 if the motif never occurred in the target graph;
	/// 3) a value in the range (0,1) representing the p-value of the given motif.
	///</returns>
	double getPValue(const std::string& label, stats_data& data)
	{
		// if a label appears in the target graph that didn't show up in any
		// random graphs, clearly it's a network motif. This scenario shouldn't
		// happen for a reasonable number of random graphs
		if (data.randomGraphRelFreqs->count(label) == 0)
		{
			return 0.0;
		} // end if

		// This shouldn't happen with the current design, but in case the design
		// changes to include functionality to display all labels found in the
		// random graphs instead of just those in the target graph, this will
		// ensure those labels are not identified as network motifs
		if (data.targetGraphRelFreqs->count(label) == 0)
		{
			return 1.0;
		} // end if

		double targetFreq = (*(data.targetGraphRelFreqs))[label];

		// TODO count_if available in clang, check g++
		std::size_t prePValue = std::count_if((*(data.randomGraphRelFreqs))[label].begin(), (*(data.randomGraphRelFreqs))[label].end(), [&](auto& x) { return x > targetFreq; });

		return static_cast<double>(prePValue) / static_cast<double>((*(data.randomGraphRelFreqs))[label].size());
	} // end method getPValue


	///<summary>
	///   Calculates the z-score for the motif-candidate <paramref name="label"/>.
	///</summary>
	///<param name="label">The label for which the z-score should be calculated.</param>
	///<param name="mean">The mean that should be used for z-score calculation.</param>
	///<param name="stdDev">The standard deviation that should be used for z-score calculation.</param>
	///<param name="stats_data"><see cref="Statistical_Analysis::stats_data"/> object with statistical data for z_score calculations.</param>
	///<returns>
	/// 1) 0 if the <paramref name="stdDev"/> is 0;
	/// 2) The z-score associated with the motif-candidate <paramref name="label"/>.
	///</returns>
	double getZScore(const std::string& label, double mean, double stdDev, stats_data& data)
	{
		double targetGraphFreq = data.targetGraphRelFreqs->count(label) == 0 ? 0.0 : (*(data.targetGraphRelFreqs))[label];
		return (stdDev == 0 ? 0.0 : (targetGraphFreq - mean) / stdDev);
	} // end method getZScore


	///<summary>
	///   Calculates the z-score for the motif-candidate <paramref name="label"/>.
	///</summary>
	///<param name="label">The label for which the z-score should be calculated.</param>
	///<param name="stats_data"><see cref="Statistical_Analysis::stats_data"/> object with statistical data for z_score calculations.</param>
	///<returns>
	/// 1) 0 if the calculated standard deviation is 0;
	/// 2) The z-score associated with the motif-candidate <paramref name="label"/>.
	///</returns>
	double getZScore(const std::string& label, stats_data& data)
	{
		double randMean = calcRandMean(label, data);
		return getZScore(label, randMean, calcRandStdDev(label, randMean, data), data);
	} // end method getZScore


	///<summary>
	///   Calculates the standard deviated for <paramref name="label"/> with the given mean.
	///</summary>
	///<param name="label">The label for which the z-score should be calculated.</param>
	///<param name="randMean">The mean of the distribution.</param>
	///<param name="stats_data"><see cref="Statistical_Analysis::stats_data"/> object with statistical data for standard deviation calculations.</param>
	///<returns>
	/// 1) 0 if the calculated standard deviation is 0;
	/// 2) The z-score associated with the motif-candidate <paramref name="label"/>.
	///</returns>
	double calcRandStdDev(const std::string& label, double randMean, stats_data& data)
	{
		double variance = get_vector_sum((*(data.randomGraphRelFreqs))[label].begin(), (*(data.randomGraphRelFreqs))[label].end(), 0.0,
			[&](auto prev, auto cur) { return prev + std::pow((cur - randMean), 2); });

		return std::sqrt(variance / (data.randGraphCount - 1));
	} // end method calcRandStdDev


	///<summary>
	/// Calculates the random mean for <paramref name="label"/>.
	///</summary>
	///<param name="label">The label for which the z-score should be calculated.</param>
	///<param name="stats_data"><see cref="Statistical_Analysis::stats_data"/> object with statistical data for mean calculations.</param>
	///<returns>
	/// The random mean associated with the motif-candidate <paramref name="label"/>.
	///</returns>
	double calcRandMean(const std::string& label, stats_data& data)
	{
		std::vector<double>* relFreqs = &(*(data.randomGraphRelFreqs))[label];

		double total = get_vector_sum(relFreqs->begin(), relFreqs->end(), 0.0);

		return (total / static_cast<double>(relFreqs->size()));
	} // end method calcRandMean


	std::ostream& operator<<(std::ostream& out, stats_data& stats)
	{
		out << "Label\tRelFreq\t\tMean\t\tStDev\t\tZ-Score\t\tP-Value\n";
		out.precision(3);

		for (auto& p : *(stats.randomGraphRelFreqs))
		{
			auto& label = p.first;
			out << label << "\t";
			if ((*(stats.targetGraphRelFreqs)).count(label) > 0)
			{
				out << std::fixed << (*(stats.targetGraphRelFreqs))[label] * 100.0;
			}
			else
			{
				out << 0.000;
			}
			out << "%\t\t";
			double mean = calcRandMean(label, stats);
			out << mean * 100 << "%\t\t";
			double stDev = calcRandStdDev(label, mean, stats);
			out << stDev << "\t\t";
			double zScore = getZScore(label, mean, stDev, stats);
			out << zScore << "\t\t";
			double pValue = getPValue(label, stats);
			out << pValue << "\n";
		}

		return out;
	}


} // end namespace Statistical_Analysis

#endif
