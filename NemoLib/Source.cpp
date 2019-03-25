#include "Config.hpp"
#include "Utility.hpp"
#include "Graph.h"
#include "SubgraphCount.hpp"
#include "SubgraphProfile.h"
#include "Stats.hpp"
#include <chrono>
#include <string>
#include <iostream>

#if _USE_THREAD_POOL
	#include <ThreadPool.hpp>
	#include "ESU_Parallel.hpp"
	#include "Parallel_RandGraphAnalysis.hpp"
#else
	#include "ESU.h"
	#include "RandomGraphAnalysis.h"
#endif


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


void display_help(string _name)
{
	std::cout << "Usage:" << std::endl;
	std::cout << "\t" << _name << " [file path] [# threads] [motif size] [# random graphs]" << std::endl;
	std::cout << "\t\t[file path]       -- complete or relative path to graph (g6 or d6 formatted) file." << std::endl;
	std::cout << "\t\t[# threads]       -- number of threads to use (ignored for sequential nemolib)." << std::endl;
	std::cout << "\t\t[motif size]      -- size of motif to search for." << std::endl;
	std::cout << "\t\t[# random graphs] -- number of random graphs to use for ESU." << std::endl;
	std::cout << "\t\t[--h | --help]    -- use instead of file path to display this help menu." << std::endl;
} // end method display_help


int main(int argc, char** argv)
{
	if(argc > 1 && (string(argv[1]) == "--h" || string(argv[1]) == "--help"))
	{
		display_help(argv[0]);
		return 0;
	} // end if

	const string filename = argc > 1 ? argv[1] : "exampleGraph.txt";
	const std::size_t n_threads = argc > 2 ? atoi(argv[2]) : 16;
	const std::size_t motifSize = argc > 3 ? atoi(argv[3]) : 4;
	const std::size_t randomCount = argc > 4 ? atoi(argv[4]) : 1000;

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
#else
	ESU::enumerate(targetg, dynamic_cast<SubgraphEnumerationResult*>(&subc), motifSize);
#endif
	unordered_map<graph64, double> targetLabelRelFreqMap(std::move(subc.getRelativeFrequencies()));

	cout << "Analyzing random graphs..." << endl << endl;

#if _USE_THREAD_POOL
	unordered_map<graph64, vector<double>> randLabelRelFreqsMap = std::move(Parallel_Analysis::analyze(targetg, randomCount, motifSize, probs, &my_pool));
	// alert all threads to terminate to save resources
	my_pool.Kill_All();
#else
	unordered_map<graph64, vector<double>> randLabelRelFreqsMap = std::move(RandomGraphAnalysis::analyze(targetg, randomCount, motifSize, probs));
#endif

	cout << "Comparing target graph to random graphs ... " << endl << endl;

	Statistical_Analysis::stats_data data{&targetLabelRelFreqMap, &randLabelRelFreqsMap, randomCount};

	auto end = _Clock::now();

	cout << endl << data << endl;

	cout << "Time = " << chrono_duration<milliseconds>(begin, end) << " milliseconds." << endl;
	cout << "Time = " << chrono_duration<seconds>(begin, end) << " seconds." << endl;

	return (EXIT_SUCCESS);
}
