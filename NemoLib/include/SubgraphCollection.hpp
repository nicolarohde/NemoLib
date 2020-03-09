#pragma once

#ifndef __NEMOLIB_SUBGRAPH_COLLECTION_HPP
#define __NEMOLIB_SUBGRAPH_COLLECTION_HPP

#include <fstream>            // ofstream 
#include <mutex>              // mutex
#include <queue>              // queue
#include <thread>             // thread
#include <unordered_map>      // unordered_map

#include "Subgraph.hpp"       // Subgraph
#include "NautyLink.hpp"      // NautyLink
#include "SubgraphCount.hpp"  // SubgraphCount
#include "Stats.hpp"          // stats_data, getPValue


class SubgraphCollection : public SubgraphCount
{
public:
	// Constructors
    SubgraphCollection() 
     : m_b_generate_subgraph_collection(false) 
    { }

	SubgraphCollection(const bool k_b_GENERATE_SUBGRAPH_COLLECTION_) 
     : m_b_generate_subgraph_collection(k_b_GENERATE_SUBGRAPH_COLLECTION_) 
    { }

	SubgraphCollection(const SubgraphCollection& other) 
    { 
        // delegate to operator=
        *this = other; 
    }

	SubgraphCollection(SubgraphCollection&& other) 
    { 
        // delegate to operator=
        *this = std::move(other);
    }

	SubgraphCollection& operator=(const SubgraphCollection& other)
    {
        m_queue_write_nemo     = other.m_queue_write_nemo;
        m_queue_write_subgraph = other.m_queue_write_subgraph;
        
        m_b_generate_subgraph_collection = other.m_b_generate_subgraph_collection;

        labelToSubgraph = other.labelToSubgraph;

        labelFreqMap = other.labelFreqMap;

        return *this;
    }

	SubgraphCollection& operator=(SubgraphCollection&& other)
    {
        m_queue_write_nemo     = std::move(other.m_queue_write_nemo);
        m_queue_write_subgraph = std::move(other.m_queue_write_subgraph);
        
        // no need to ever move a bool
        m_b_generate_subgraph_collection = other.m_b_generate_subgraph_collection;

        labelToSubgraph = std::move(other.labelToSubgraph);

        labelFreqMap = std::move(other.labelFreqMap);

        return *this;
    }

	virtual ~SubgraphCollection() 
    {
        if(true == m_thread_write_nemo_thread.joinable())
        {
            m_thread_write_nemo_thread.join();
        } // end if
        else if (false == m_queue_write_nemo.empty())
        {
            std::cerr << "SubgraphCollection destructor invoked with non-empty network motif buffer. Did you forget to call write_nemo_collection?" << std::endl;
        } // end elif

        if(true == m_thread_write_subgraph_thread.joinable())
        {
            m_thread_write_subgraph_thread.join();
        } // end if
        else if (false == m_queue_write_nemo.empty())
        {
            std::cerr << "SubgraphCollection destructor invoked with non-empty subgraph buffer. Did you forget to call write_subgraph_collection?" << std::endl;
        } // end elif
    } // end Destructor


	/** @brief Adds the given graph to the frequency table and the subgraphb collection (if requested). 
      * @param currentSubgraph The subgraph to add to the collection
      * @param nautylink Object used for getting the canonical label of currentSubgraph
      */
	void add(Subgraph& currentSubgraph, NautyLink& nautylink) override
	{
        //std::cerr << "In SubgraphCollection::add " << std::endl;

        std::string label = nautylink.nautylabel_helper(currentSubgraph);

        //std::cerr << "Got label: " << label << std::endl;

        // delegate to base class for frequencies
		SubgraphCount::add(currentSubgraph, nautylink, label);

        //std::cerr << "labelToSubgraph length before: " << labelToSubgraph.size() << std::endl;

		labelToSubgraph[label].push_back(currentSubgraph);

        //std::cerr << "labelToSubgraph length after: " << labelToSubgraph.size() << std::endl;

        if (true == m_b_generate_subgraph_collection)
        {
            std::lock_guard<std::mutex> guard(m_mtx_write_subgraph_q);
    		m_queue_write_subgraph.push(std::string{label + "\n" + static_cast<std::string>(currentSubgraph) + "\n"});
        } // end if
	} // end method add


    /** @brief Finds and adds all network motifs with p-value <= 0.05 to the nemo buffer.
      *        The buffer is only written to the file once write_nemo_collection is invoked
      *        or flush is called, in which case subgraphs may also be written (if collected). 
      * @param data Statistical data found from random graph analysis
      */
    void find_network_motifs(const Statistical_Analysis::stats_data& data)
    {
        //std::cerr << "Size of labelToSubgraph is: " << labelToSubgraph.size() << std::endl;

        for (const auto& p : labelToSubgraph)
		{
            //std::cerr << "Calculating p value for label " << p.first << " ..." << std::endl;

			if (Statistical_Analysis::getPValue(p.first, data) <= 0.05)
			{
                //std::cerr << "The label " << p.first << " has p value less than or equal to 0.05" << std::endl;
                //std::cerr << "Adding " << p.second.size() << " motifs to queue" << std::endl;

                std::lock_guard<std::mutex> guard(m_mtx_write_nemo_q);

				for (const auto& q : p.second)
				{
                    m_queue_write_nemo.push(std::string{p.first + "\n" + static_cast<std::string>(q) + "\n"});
				} // end for q
			} // end if
		} // end for p
    } // end method find_network_motifs


    /** @brief Writes the network motif buffer to the nemo collection file
      * @param kr_str_NEMO_PATH_ Path to write any remaining network motifs to
      * @param k_b_BLOCK_ If true, the function will block until writing is completed
      *                   instead of spawning a worker thread and returning immediately
      */
	void write_nemo_collection(const std::string& kr_str_NEMO_PATH_, const bool k_b_BLOCK_ = false)
	{
        // don't generate new data while writing
        if(true == m_thread_write_nemo_thread.joinable())
        {
            m_thread_write_nemo_thread.join();
        }

        if (false == k_b_BLOCK_)
        {
            m_thread_write_nemo_thread = std::thread(
                [this, kr_str_NEMO_PATH_]()
                {
                    this->write_nemo_collection_helper(kr_str_NEMO_PATH_);
                } // end lambda
            ); // end std::thread
        } // end if
        else
        {
            write_nemo_collection_helper(kr_str_NEMO_PATH_);
        } // end else 
	}

    /** @brief Writes the subgraph buffer to the subgraph collection file 
      * @param kr_str_SUBGRAPH_PATH_ Path to write any remaining subgraphs to
      * @param k_b_BLOCK_ If true, the function will block until writing is completed
      *                   instead of spawning a worker thread and returning immediately
      */
    void write_subgraph_collection(const std::string& kr_str_SUBGRAPH_PATH_, const bool k_b_BLOCK_ = false)
    {
        if(true == m_thread_write_subgraph_thread.joinable())
        {
            m_thread_write_subgraph_thread.join();
        }

        if (false == k_b_BLOCK_)
        {
            m_thread_write_subgraph_thread = std::thread(
                [this, kr_str_SUBGRAPH_PATH_]()
                {
                    this->write_subgraph_collection_helper(kr_str_SUBGRAPH_PATH_);
                } // end lambda
            ); // end std::thread
        } // end if
        else
        {
            write_subgraph_collection_helper(kr_str_SUBGRAPH_PATH_);
        } // end else 
    } // end method write_subgraph_collection


    /** @brief Blocks until all buffers have been flushed to files.
      * @param kr_str_SUBGRAPH_PATH_ Path to write any remaining subgraphs to
      * @param kr_str_NEMO_PATH_ Path to write any remaining network motifs to
      */
    void flush(const std::string& kr_str_SUBGRAPH_PATH_, const std::string& kr_str_NEMO_PATH_)
    {
        if(true == m_thread_write_nemo_thread.joinable())
        {
            m_thread_write_nemo_thread.join();
        } // end if
        else if (false == m_queue_write_nemo.empty())
        {
            write_nemo_collection_helper(kr_str_NEMO_PATH_);
        } // end elif

        if(true == m_thread_write_subgraph_thread.joinable())
        {
            m_thread_write_subgraph_thread.join();
        } // end if
        else if (false == m_queue_write_nemo.empty())
        {
            write_subgraph_collection_helper(kr_str_SUBGRAPH_PATH_);
        } // end elif
    } // end method flush

    inline SubgraphCollection operator+(const SubgraphCollection& RHS)
	{
		SubgraphCollection out(*this);
		out += RHS;
		return out;
	}


	inline SubgraphCollection& operator+=(const SubgraphCollection& RHS)
	{
		for (auto& p : RHS.labelFreqMap)
		{
			if (labelFreqMap.count(p.first) == 0)
			{
				labelFreqMap[p.first] = p.second;
			}
			else
			{
				labelFreqMap[p.first] += p.second;
			}
		}

        for (auto& p : RHS.labelToSubgraph)
		{
			if (0 == labelToSubgraph.count(p.first))
			{
				labelToSubgraph[p.first] = p.second;
			}
			else
			{
                auto* v = &labelToSubgraph[p.first];
                v->reserve(v->size() + p.second.size());
				v->insert(v->end(), p.second.begin(), p.second.end());
			}
		}

		return *this;
	}


protected:
    /** @brief Writes the network motif buffer to the given file
      * @param kr_str_NEMO_PATH_ Path to write all network motifs to
      * @remarks Note that the parameter should not be a reference to 
      *          avoid a temporary reference being given to a thread
      *          that will outlive the calling scope. 
      */
    void write_nemo_collection_helper(const std::string k_str_NEMO_PATH_)
    {
        std::ofstream of_nemo_file(k_str_NEMO_PATH_);

        { // lock_guard scope
            std::lock_guard<std::mutex> guard(m_mtx_write_nemo_q);
            while(false == m_queue_write_nemo.empty())
            {
                of_nemo_file << m_queue_write_nemo.front();
                m_queue_write_nemo.pop();
            } // end while
        } // end lock_guard
    } // end method write_nemo_collection


    /** @brief Writes the subgraph buffer to the given file
      * @param kr_str_SUBGRAPH_PATH_ Path to write all subgraphs to
      * @remarks Note that the parameter should not be a reference to 
      *          avoid a temporary reference being given to a thread
      *          that will outlive the calling scope. 
      */
    void write_subgraph_collection_helper(const std::string kr_str_SUBGRAPH_PATH_)
    {
        std::ofstream of_subgraph_file(kr_str_SUBGRAPH_PATH_);

        { // lock_guard scope
            std::lock_guard<std::mutex> guard(m_mtx_write_subgraph_q);
            while(false == m_queue_write_subgraph.empty())
            {
                of_subgraph_file << m_queue_write_subgraph.front();
                m_queue_write_subgraph.pop();
            } // end while
        } // end lock_guard
    } // end method write_subgraph_collection_helper

    bool m_b_generate_subgraph_collection;
    
    //! write thread for subgraph collection queue
    std::thread m_thread_write_subgraph_thread;       
    //! write thread for nemo collection queue
    std::thread m_thread_write_nemo_thread;           

    //! protects subgraph queue
    mutable std::mutex m_mtx_write_subgraph_q;    
    //! protects nemo queue
    mutable std::mutex m_mtx_write_nemo_q;            
    
    //! buffer for subgraphs to be written to the file
    std::queue<std::string> m_queue_write_subgraph;   
    //! buffer for network motifs to be written to the file
    std::queue<std::string> m_queue_write_nemo;       

    //! stores each motif label with all instances of that motif
	std::unordered_map<std::string, std::vector<Subgraph>> labelToSubgraph; 
}; // end class SubgraphCollection

#endif /* __NEMOLIB_SUBGRAPH_COLLECTION_HPP */