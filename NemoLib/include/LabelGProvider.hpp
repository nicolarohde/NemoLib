
#pragma once

#ifndef __LABEL_G_PROVIDER_HPP
#define __LABEL_G_PROVIDER_HPP

#include <queue>         // queue
#include <mutex>         // mutex, lock_guard
#include <utility>       // pair
#include <string>        // string
#include <thread>        // thread
#include <vector>        // vector
#include <functional>    // function
#include <unordered_map> // unordered_map
#include <iostream>      // cerr
#include <cstddef>       // size_t
#include <stdio.h>       // FILE, popen, pclose, fgets

class LabelGProvider
{
public:
    //! the callback type used to return the cannonical label
    using callback_t = std::function<void(std::string)>;


    /** @brief Terminates the worker and destroys the object.*/
    ~LabelGProvider(void) noexcept
    {
        // inform thread to terminate
        m_mtx_terminate.lock();
        m_queue_terminate.push(true);
        m_mtx_terminate.unlock();

        if (true == m_thread_worker.joinable())
        {
            m_thread_worker.join();
        } // end if
    } // end Destructor


    /** @brief Starts the worker thread.
      * @param kr_str_LABELG_PATH_ Path to the labelg program 
      * @remark This function should not be invoked multiple times 
      *         as this will cause threads to be overwritten.
      */
    void start_up(const std::string& kr_str_LABELG_PATH_)
    {
        m_str_labelg_path = kr_str_LABELG_PATH_;

        // if this is a call to start_up after a failure
        // there will still be a failure signal in the queue
        while (false == m_queue_terminate.empty())
        {
            m_queue_terminate.pop();
        } // end if

        m_thread_worker = std::thread(
            [this]{
                this->loop();
            }
        );
    } // end Constructor


    /** @brief Add a new string to send to labelg
      * @param kr_str_LABEL_ The label to convert to to cannonical label
      * @param job_CALLBACK_ The callback function to return the cannonical labe
      * @remarks The caller should get a future to the job_CALLBACK_ function
      * as it will be invoked with the cannonical label. The function will be
      * invoked with the cannonical label associated with kr_str_LABEL_
      */
    void add_job(const std::string& kr_str_LABEL_, callback_t job_CALLBACK_)
    {
        std::lock_guard guard(m_mtx_jobs);
        m_queue_jobs.push(std::make_pair(kr_str_LABEL_, job_CALLBACK_));
    } // end method add_job


    /** @brief Checks if the worker thread is still running or whether it has
     *         terminated since the last call to @see start_up.
     *  @remark Note that this function will only return false if the thread
     *          crashed as the destructor will signal the thread to terminate
     *          in the normal case. 
     */
    bool is_worker_still_running(void)
    {
        return false == m_queue_terminate.empty();
    } // end method is_thread_still_running

private:
    /** @brief Main loop of worker threads. Consumes jobs in the queue
     *         until a terminate signal is pushed into the terminate queue.
     *  @remark If the Popen call fails, it will discard the current job
     *          and terminate. The callback will never be invoked in that 
     *          case which means this error may be fatal.
     */
    void loop(void)
    {
        // size of the buffer used to communicate with labelg
        constexpr std::size_t ku_li_buffer_size = 64;

        // queues are thread-safe, there might be a nicer way
        // to signal a terminate, but it doesn't really matter
        while(m_queue_terminate.empty() == true)
        {   
            std::pair<std::string, callback_t> job;

            if (m_queue_jobs.empty() == true)
            {
                // nothing to do
                std::this_thread::yield();
                continue;
            } // end if
            else
            {
                // get the next job
                std::lock_guard guard(m_mtx_jobs);
                job = std::move(m_queue_jobs.front());
                m_queue_jobs.pop();
            } // end else

            // sending an empty string to labelg will probably
            // cause some unwanted side-effects like crashing
            if (false == job.first.empty())
            {
                if(1 == m_umap_memo_table.count(job.first))
                {
                    // cannonical label is memoized, no need to popen
                    job.second(m_umap_memo_table[job.first]);
                } // end if
                else
                {
                    //! c-style file for popen
                    FILE* fp;
                    
                    //! buffer to read popen pipe 
                    char p_buffer[ku_li_buffer_size];

                    //! the command to run labelg
                    std::string str_cmd{m_str_labelg_path};
                    //! the label returned by labelg
                    std::string str_cannonical_label;
                    //! the string read from the popen output
                    std::string str_output{""};

                    // add the calculated label to the command
                    str_cmd += " '";
                    str_cmd += job.first;
                    str_cmd += "'";

                    // if popen fails there really is no way to recover
                    // in that case we will inform the user and exit 
                    // pushing false into the queue allows the user to 
                    // know that something went wrong via the 
                    // is_worker_still_running function 
                    // however, the callback won't be invoked, so there
                    // will likely be a thread waiting forever for this
                    if ((fp = popen (str_cmd.c_str(), "r")) == NULL)
                    {
                        std::lock_guard guard(m_mtx_terminate);
                        std::cerr << "call to popen failed!" << std::endl;
                        std::cerr << "command was: '" << str_cmd << "'" << std::endl;
                        m_queue_terminate.push(false);
                        exit(1);
                    } // end if

                    // read the first string from the buffer
                    while (fgets(p_buffer, sizeof(p_buffer), fp)) 
                    {
                        str_output += p_buffer;
                    } // end while

                    str_cannonical_label = str_output.substr(0, str_output.size() - 1);
                    pclose(fp);

                    // memoize cannonical label
                    m_umap_memo_table[job.first] = str_cannonical_label;

                    // invoke the callback with the return value                
                    job.second(str_cannonical_label);
                } // end else
            } // end if
        } // end while
    } // end method loop

    //! worker thread that will run the main loop
    std::thread m_thread_worker;

    //! path to the labelg executable
    std::string m_str_labelg_path;

    //! queue to receive work
    std::queue<std::pair<std::string, callback_t>> m_queue_jobs;
    //! used to signal thread to terminate
    std::queue<bool> m_queue_terminate;

    //! memoization table for motifs
    std::unordered_map<std::string, std::string> m_umap_memo_table;

    //! mutex protecting job queue
    std::mutex m_mtx_jobs;
    //! mutex protecting terminate queue
    std::mutex m_mtx_terminate;
}; // end class LabelGProvider

#endif // !__LABEL_G_PROVIDER_HPP
