#include "Config.hpp"
#include "Utility.hpp"
#include "Graph.hpp"
#include "SubgraphCount.hpp"
#include "SubgraphProfile.hpp"
#include "Stats.hpp"
#include <chrono>
#include <string>
#include <iostream>

#include "loguru.hpp"

#include "ThreadPool.hpp"
#include "ESU_Parallel.hpp"
#include "Parallel_RandGraphAnalysis.hpp"


using std::unordered_map;
using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::chrono::seconds;
using std::chrono::milliseconds;


void display_help(string _name)
{
	std::cout << "Usage:" << std::endl;
	std::cout << "\t" << _name << " [file path] [# threads] [motif size] [# random graphs] [labelg path]" << std::endl;
	std::cout << "\t\t[file path]       -- complete or relative path to graph (g6 or d6 formatted) file." << std::endl;
	std::cout << "\t\t[# threads]       -- number of threads to use (ignored for sequential nemolib)." << std::endl;
	std::cout << "\t\t[motif size]      -- size of motif to search for." << std::endl;
	std::cout << "\t\t[# random graphs] -- number of random graphs to use for ESU." << std::endl;
	std::cout << "\t\t[labelg path]     -- path to the labelg program to use." << std::endl;
	std::cout << "\t\t[-h | --help]     -- use instead of [file path] to display this help menu." << std::endl;
} // end method display_help


int main(int argc, char** argv)
{
    if(argc > 6 || (argc > 1 && (string(argv[1]) == "-h" || string(argv[1]) == "--help")))
	{
		display_help(argv[0]);
		return argc > 6;
	} // end if

    // turn on logging 
    // -v option can be used to change verbosity
    loguru::init(argc, argv);

	const string filename = argc > 1 ? argv[1] : "./test/exampleGraph.txt";
	const std::size_t n_threads = argc > 2 ? atoi(argv[2]) : 16;
	const std::size_t motifSize = argc > 3 ? atoi(argv[3]) : 4;
	const std::size_t randomCount = argc > 4 ? atoi(argv[4]) : 1000;
	const string labelg_path = argc > 5 ? argv[5] : "./labelg";

	SubgraphCount subc;
	vector<double> probs(motifSize - 2, 1.0);
	probs.insert(probs.end(), { 0.5, 0.5 });

	// initialize thread pool
	ThreadPool my_pool(n_threads);

	// alert all threads to start
	my_pool.Start_All_Threads();

	auto begin = _Clock::now();

	Graph targetg(filename, false);

    LOG_F(INFO, "Enumerating graph ...");

	ESU_Parallel::enumerate<SubgraphCount>(targetg, &subc, static_cast<int>(motifSize), &my_pool, labelg_path);

    LOG_F(INFO, "Done Enumerating. Getting relative frequencies ...");

	unordered_map<std::string, double> targetLabelRelFreqMap(std::move(subc.getRelativeFrequencies()));

    LOG_F(INFO, "Analyzing random graphs...\n");


	Parallel_Analysis::AnalyzeArgPack analyze_args
	(
		&targetg, randomCount, motifSize, probs, &my_pool, labelg_path
	);

	auto randLabelRelFreqsMap = std::move(Parallel_Analysis::analyze(analyze_args));
	
	// alert all threads to terminate
	my_pool.Kill_All();

    LOG_F(INFO, "Comparing target graph to random graphs ...\n");

	Statistical_Analysis::stats_data data{&targetLabelRelFreqMap, &randLabelRelFreqsMap, randomCount};

	auto end = _Clock::now();

	cout << endl << data << endl;

    LOG_F(INFO, "Time = %f.4 milliseconds", chrono_duration<milliseconds>(begin, end));
    LOG_F(INFO, "Time = %f.4 seconds", chrono_duration<seconds>(begin, end));
} // end Main
