
#pragma once

#ifndef __LABEL_G_PROVIDER_HPP
#define __LABEL_G_PROVIDER_HPP

#include <queue>
#include <mutex>
#include <utility>
#include <string>
#include <future>
#include <thread>
#include <vector>
#include <functional>
#include <unordered_map>
#include <iostream>
#include <stdio.h>

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

        if (m_thread_worker.joinable() == true)
        {
            m_thread_worker.join();
        } // end if
    } // end Destructor

    /** @brief Starts the worker thread.
      * @param kr_str_LABELG_PATH_ Path to the labelg program 
      */
    void start_up(const std::string& kr_str_LABELG_PATH_)
    {
        m_str_labelg_path = kr_str_LABELG_PATH_;

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
      * as it will be invoked with the cannonical label. The function should 
      * simply return the string parameter it receives. 
      */
    void add_job(const std::string& kr_str_LABEL_, callback_t job_CALLBACK_)
    {
        std::lock_guard guard(m_mtx_jobs);
        m_queue_jobs.push(std::make_pair(kr_str_LABEL_, job_CALLBACK_));
    } // end method add_job

private:

    void loop(void)
    {
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
            if (job.first.empty() == false)
            {
                if(m_umap_memo_table.count(job.first) == 1)
                {
                    // cannonical label is memoized, no need to popen
                    job.second(m_umap_memo_table[job.first]);
                } // end if
                else
                {
                    std::string cmd = m_str_labelg_path;
                    cmd += " \"";
                    cmd += job.first;
                    cmd += "\"";

                    FILE* fp;
                    const int sizebuf = 64;
                    char buff[sizebuf];
                    auto out = std::vector<std::string>();

                    if ((fp = popen (cmd.c_str(), "r")) == NULL)
                    {
                        std::cerr << "call to popen failed!" << std::endl;
                        exit(1);
                    }

                    std::string cur_string = "";
                    while (fgets(buff, sizeof (buff), fp)) 
                    {
                        cur_string += buff;
                    }

                    out.push_back(cur_string.substr (0, cur_string.size() -1));
                    pclose(fp);

                    // memoize cannonical label
                    m_umap_memo_table[job.first] = out[0];

                    // invoke the callback with the return value                
                    job.second(out[0]);
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
};

#endif // !__LABEL_G_PROVIDER_HPP
