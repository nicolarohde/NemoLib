#include "Config.hpp"
#include "Utility.hpp"
#include "Graph.hpp"
#include "SubgraphCollection.hpp"
#include "Stats.hpp"

#include "loguru.hpp"

#include <chrono>
#include <string>
#include <iostream>
#include <memory>

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


/*
 * Simple C++ Test Suite
 */

void display_help(const string& kr_str_NAME_)
{
	std::cout << "Usage:" << std::endl;
	std::cout << kr_str_NAME_ << "[OPTIONS] [file path] [# threads] [motif size] [# random graphs] [labelg path] [output path]" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "\t[-v VERBOSITY]    -- set the logging verbosity level. Integer in range [0,4]." << std::endl;
	std::cout << "\t[-h | --help]     -- use instead of [file path] to display this help menu." << std::endl;
	std::cout << std::endl;
	std::cout << "Positional :" << std::endl;
	std::cout << "\t[file path]       -- complete or relative path to graph (g6 or d6 formatted) file." << std::endl;
	std::cout << "\t[# threads]       -- number of threads to use (Note that Nemolib uses 2 additional threads)." << std::endl;
	std::cout << "\t[motif size]      -- size of motif to search for." << std::endl;
	std::cout << "\t[# random graphs] -- number of random graphs to use for ESU." << std::endl;
	std::cout << "\t[labelg path]     -- path to the special labelg binary." << std::endl;
	std::cout << "\t[output path]     -- path where to store the nemo collection." << std::endl;
} // end method display_help


int main(int argc, char** argv)
{
    // check if user wants to see the help
    // or if they gave too many parameters
	if(argc > 1 && (string(argv[1]) == "-h" || string(argv[1]) == "--help"))
	{
		display_help(argv[0]);
		return 0;
	} // end if

    if (argc > 7)
    {
        std::cerr << "Received " << argc << " arguments but only wanted 7" << std::endl;
        display_help(argv[0]);
        return 1;
    }

    // turn on logging 
    // -v option can be used to change verbosity
    loguru::init(argc, argv);

    // get parameters or use default values
	const string      filename    = argc > 1 ?      argv[1]  : "./test/exampleGraph.txt";
	const std::size_t n_threads   = argc > 2 ? atoi(argv[2]) : 14;
	const std::size_t motifSize   = argc > 3 ? atoi(argv[3]) : 3;
	const std::size_t randomCount = argc > 4 ? atoi(argv[4]) : 1000;
	const string      labelg_path = argc > 5 ?      argv[5]  : "./labelg";
	const string      nemoc_path  = argc > 6 ?      argv[6]  : "./test/nemocollection.txt";

    auto subc = std::make_unique<SubgraphCollection>(false);

	vector<double> probs(motifSize - 2, 1.0);
	probs.insert(probs.end(), { 0.5, 0.5 });

	// initialize thread pool
	ThreadPool my_pool(n_threads);

	// alert all threads to start
	my_pool.Start_All_Threads();

    // start execution timer
	auto begin = _Clock::now();

    LOG_F(INFO, "Reading in the graph in file %s", filename);

    // read in the given graph as undirected 
	Graph targetg(filename, false);

	LOG_F(INFO, "Enumerating graph ...");

	ESU_Parallel::enumerate<SubgraphCollection>(targetg, subc.get(), static_cast<int>(motifSize), &my_pool, labelg_path);
	unordered_map<std::string, double> targetLabelRelFreqMap(std::move(subc->getRelativeFrequencies()));

	LOG_F(INFO, "Analyzing random graphs...");

	Parallel_Analysis::AnalyzeArgPack analyze_args
	(
		&targetg, randomCount, motifSize, probs, &my_pool, labelg_path
	);

	auto randLabelRelFreqsMap = std::move(Parallel_Analysis::analyze(analyze_args));
	
	// alert all threads to terminate to save resources
	my_pool.Kill_All();

	LOG_F(INFO, "Comparing target graph to random graphs ... ");

	Statistical_Analysis::stats_data data{&targetLabelRelFreqMap, &randLabelRelFreqsMap, randomCount};

	LOG_F(INFO, "Finding motifs ...");

	subc->find_network_motifs(data);

	auto end = _Clock::now();

	LOG_F(INFO, "Writing motifs to file ...");

	subc->write_nemo_collection(nemoc_path, false);

	LOG_F(INFO, "Done writing motifs");

	cout << endl << data << endl;

	LOG_F(INFO, "Time = %.4f milliseconds", chrono_duration<milliseconds>(begin, end));
    LOG_F(INFO, "Time = %.4f seconds", chrono_duration<seconds>(begin, end));

	subc->flush("", nemoc_path);

	return (EXIT_SUCCESS);
}
