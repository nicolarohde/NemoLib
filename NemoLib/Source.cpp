#include "Config.hpp"
#include <iostream>
#include "Graph.h"
#include "SubgraphCount.h"
#include "SubgraphProfile.h"
#include "StatisticalAnalysis.h"
#include "RandomGraphAnalysis.h"
#include <chrono>
#include <ctime>
#include <string>

#if _USE_THREAD_POOL
#include <ThreadPool.hpp>
#include "ESU_Parallel.hpp"
#else
#include "ESU.h"
#endif

// Typedefs to make the clock and timepoint names shorter
typedef std::chrono::high_resolution_clock	_Clock;
#define CHRONO_DURATION(A, B) std::chrono::duration_cast<B>((A)).count()
#define DURATION_IN_NANOS(A) CHRONO_DURATION(A, std::chrono::nanoseconds)
#define DURATION_IN_MICROS(A) CHRONO_DURATION(A, std::chrono::microseconds)
#define DURATION_IN_MILLIS(A) CHRONO_DURATION(A, std::chrono::milliseconds)
#define DURATION_IN_SECS(A) CHRONO_DURATION(A, std::chrono::seconds)

template <typename D, typename T>
inline double chrono_duration(T start, T end)
{
	return std::chrono::duration_cast<D>(end - start).count();
}


//#pragma comment(lib,"NemoLib.lib")		// declare use of nemolib library to linker

using std::unordered_map;
using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::chrono::seconds;
using std::chrono::milliseconds;


/*
 * Simple C++ Test Suite
 */

template<typename T>
void printmap(const T& _map) 
{
	for (const auto& p : _map)
		cout << p.first << " => " << p.second << endl;
}


int main(int argc, char** argv)
{
	constexpr std::size_t motifSize = 4;
	constexpr std::size_t randomCount = 6;
	constexpr std::size_t n_threads = 16;

	const string filename = "exampleGraph.txt";

	SubgraphCount subc;
	vector<double> probs(motifSize - 2, 1.0);
	probs.insert(probs.end(), { 0.5, 0.5 });

#if _USE_THREAD_POOL
	// initialize thread pool
	ThreadPool my_pool(n_threads);

	// alert all threads to start
	my_pool.Start_All_Threads();
#endif

	auto begin = _Clock::now();

	Graph targetg(filename, false);

#if _USE_THREAD_POOL
	ESU_Parallel::enumerate(targetg, dynamic_cast<SubgraphEnumerationResult*>(&subc), motifSize, &my_pool);
	// alert all threads to terminate to save resources
	my_pool.Kill_All();
#else
	ESU::enumerate(targetg, dynamic_cast<SubgraphEnumerationResult*>(&subc), motifSize);
#endif
	unordered_map<graph64, double> targetLabelRelFreqMap(std::move(subc.getRelativeFrequencies()));

	cout << "Analyzing random graphs..." << endl;

	unordered_map<graph64, vector<double>> randLabelRelFreqsMap = std::move(RandomGraphAnalysis::analyze(targetg, randomCount, motifSize, probs));

	cout << "Comparing target graph to random graphs" << endl;

	StatisticalAnalysis stat(targetLabelRelFreqMap, randLabelRelFreqsMap, randomCount);

	auto end = _Clock::now();

	cout << stat << endl;

	cout << "Time = " << chrono_duration<milliseconds>(begin, end) << " milliseconds." << endl;
	cout << "Time = " << chrono_duration<seconds>(begin, end) << " seconds." << endl;

	return (EXIT_SUCCESS);
}
