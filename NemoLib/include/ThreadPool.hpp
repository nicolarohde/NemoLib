#pragma once

#ifndef __THREAD_POOL_HPP
#define __THREAD_POOL_HPP

#include <type_traits>	// size_t
#include <thread>		// thread
#include <vector>		// vector
#include <mutex>		// mutex, lock_guard
#include <iostream>		// cout
#include <queue>		// queue
#include <future>		// packaged_task
#include <stdexcept>    // exception
#include <exception>    // exception_ptr

class ThreadPool
{
	using Guard_t = std::lock_guard<std::mutex>;
public:
	enum THREAD_SIGNALS
	{
		STARTING,
		WORKING,
		IDLE,
		SIGTERM,
		TERMINATING
	}; // end enum THREAD_SIGNALS

	// Disallow any kind of copy/move operation on thread pools
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool(ThreadPool&&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
	ThreadPool& operator=(ThreadPool&&) = delete;


	///<summary>
	/// Initializes the thread pool to support <paramref name="ku_li_N_THREADS_"/> threads.
	///</summary>
	///<param name="ku_li_N_THREADS_">The number of threads this pool should support.</param>
	///<remarks>
	/// The threads will not be started the moment the pool is initialized, 
	/// to start the threads, Start_All_Threads or Start_N_Threads must be invoked.
	///</remarks>
	ThreadPool(const std::size_t ku_li_N_THREADS_)
	{
		// threads must be started explicitly
		mu_li_nrunning = 0;
		mu_li_nthreads = ku_li_N_THREADS_;
		m_vect_threads.reserve(mu_li_nthreads);
	} // end Constructor(1)


	///<summary>
	/// Alerts all threads to terminate, clears the job queue, and then waits for all threads to terminate.
	///</summary>
	///<remarks>
	/// Threads will be allowed to finish before being terminated, however, if jobs are still in the job
	/// queue, they will not be executed but instead discarded.
	///</remarks>
	~ThreadPool(void)
	{
		// signal all threads to terminate
		m_mtx_signals.lock();
		std::for_each(m_vect_signals.begin(), m_vect_signals.end(),
			[&](auto& sigterm)
			{
				sigterm = THREAD_SIGNALS::SIGTERM;
			} // end lambda
		); // end foreach
		m_mtx_signals.unlock();

		Empty_Job_Queue();

		// wait for all threads to terminate
		for (auto& t : m_vect_threads)
		{
			if (t.joinable())
			{
				t.join();
			} // end if
		} // end for t
	} // end Destructor


	///<summary>
	/// Starts all threads. All threads will begin executing jobs.
	///</summary>
	///<returns>
	/// True on success, false if an issue occurs. 
	///</returns>
	///<remarks>
	/// The threads will begin executing jobs as they become available.
	///</remarks>
	bool Start_All_Threads(void)
	{
		return Start_N_Threads(mu_li_nthreads);
	} // end method 


	///<summary>
	/// Starts <paramref name="ku_li_N_THREADS_"/> threads. These threads will begin executing jobs.
	///</summary>
	///<param name="ku_li_N_THREADS_">The number of threads to start.</param>
	///<returns>
	/// True on success, false if an issue occurs. 
	///</returns>
	///<remarks>
	/// The value of <paramref name="ku_li_N_THREADS_"/> must be at least as large as 
	/// the number of running threads in order for this call to succeed.
	/// By providing an <paramref name="ku_li_N_THREADS_"/> value larger than the capacity
	/// of the thread pool, the pool can be grown in size.
	///</remarks>
	bool Start_N_Threads(const std::size_t ku_li_N_THREADS_)
	{
		if (ku_li_N_THREADS_ < mu_li_nrunning)
		{
			return false;
		} // end if
		else if (ku_li_N_THREADS_ > mu_li_nthreads)
		{
			mu_li_nthreads = ku_li_N_THREADS_;
		} // end elif

		if (mu_li_nrunning != mu_li_nthreads)
		{
			m_vect_threads.reserve(mu_li_nthreads);

			m_mtx_signals.lock();
			for (auto i = mu_li_nrunning; i < mu_li_nthreads; i++)
			{
				std::size_t my_id = i;
				m_vect_signals.push_back(THREAD_SIGNALS::STARTING);
				m_vect_threads.push_back(std::thread([this, my_id](void) {idle_thread(my_id); }));
			} // end for i

			mu_li_nrunning = m_vect_threads.size();
			m_mtx_signals.unlock();
		} // end if

		return true;
	} // end method Start_N_Threads


	///<summary>
	/// Adds the given job <paramref name="fn_job_"/> to the end of the execution queue.
	///</summary>
	///<param name="fn_job_">A ready-to-execute job that should be executed.</param>
	void Add_Job(std::function<void(void)> fn_job_)
	{
		Guard_t guard(m_mtx_tasks);

		m_q_tasks.push(fn_job_);
	} // end method Add_Job


	///<summary>
	/// Terminates all threads currently running in the thread pool. If <paramref name="b_SYNC_FIRST_"/> 
	/// is set, the pool will sychronize before terminating the running threads.
	///</summary>
	///<param name="b_SYNC_FIRST_">Whether or not to block and synchronize before terminating.</param>
	///<remarks>
	/// The job queue will not be cleared by this function, an explicit call to Empty_Job_Queue is required.
	///</remarks>
	void Kill_All(const bool b_SYNC_FIRST_ = false)
	{
		if (b_SYNC_FIRST_ == true)
		{
			Synchronize();
		} // end if

		// signal all threads to terminate
		m_mtx_signals.lock();
		std::for_each(m_vect_signals.begin(), m_vect_signals.end(),
			[&](auto& sigterm)
			{
				sigterm = THREAD_SIGNALS::SIGTERM;
			} // end lambda
		); // end foreach
		m_mtx_signals.unlock();

		// wait for all threads to terminate
		for (auto& t : m_vect_threads)
		{
			if (t.joinable() == true)
			{
				t.join();
			} // end if
		} // end for t

		m_vect_threads.clear();
		m_vect_signals.clear();

		mu_li_nrunning = 0;
	} // end method Stop


	///<summary>
	/// Removes all pending jobs from the queue and destroys them.
	///</summary>
	void Empty_Job_Queue(void)
	{
		Guard_t guard(m_mtx_tasks);
		while (m_q_tasks.empty() == false)
		{
			m_q_tasks.pop();
		} // end while
	} // end method Empty_Job_Queue


	///<summary>
	/// Blocks until all pending jobs have been completed.
	///</summary>
	///<returns>
	/// True on successful completion of all pending jobs.
	/// False if no threads are running.
	///</returns>
	bool Synchronize(void) const noexcept
	{
		if (mu_li_nrunning == 0)
		{
			std::cerr << "ThreadPool::Synchronize invoked with 0 running threads! Did you call Start_All?" << std::endl;
			return false;
		} // end if

		while (m_q_tasks.empty() == false)
		{
			std::this_thread::yield();
		} // end while

		// wait for all threads to complete their current work
		for (auto& _signal : m_vect_signals)
		{
			while (_signal == THREAD_SIGNALS::WORKING)
			{
				std::this_thread::yield();
			} // end while
		} // end for

		return true;
	} // end method Synchronize


	///<summary>
	/// Accessor for the number of threads running.
	///</summary>
	///<returns>The number of threads currently running.</returns>
	std::size_t N_Threads_Running(void) const noexcept
	{
		return mu_li_nrunning;
	} // end method N_Threads_Running


	///<summary>
	/// Accessor for the number of jobs not completed.
	///</summary>
	///<returns>The number of jobs that remain in the queue.</returns>
	std::size_t N_Jobs_Remaining(void) const noexcept
	{
		Guard_t guard(m_mtx_tasks);

		return m_q_tasks.size();
	} // end method N_Jobs_Remaining


	///<summary>
	/// Returns whether or not an exception pointer was queued for processing.
	///</summary>
	///<returns>True iff there are exceptions waiting for processing, otherwise false.</returns>
	bool Has_Exceptions(void) const noexcept
	{
		Guard_t guard(m_mtx_exception);

		return m_q_exception.empty() == false;
	} // end method Has_Exceptions


	///<summary>
	/// Returns a list of all threads and their current states at the time of invocation.
	///</summary>
	///<returns>A vector of all thread states.</returns>
	std::vector<int> Thread_States(void) const
	{
		Guard_t guard(m_mtx_signals);

		return std::vector<int>(m_vect_signals);
	} // end method Thread_States


	///<summary>
	/// Accessor for the last exception that has occurred, exceptions are returned in order of occurrence.
	///</summary>
	///<returns>An exception pointer to the next exception, or a nullptr if no exceptions are queued.</returns>
	std::exception_ptr Last_Exception(void)
	{
		Guard_t guard(m_mtx_exception);
		std::exception_ptr exptr_exception;

		if (m_q_exception.empty() == false)
		{
			exptr_exception = m_q_exception.front();
			m_q_exception.pop();
		} // end if

		return exptr_exception;
	} // end method Last_Exception


protected:
	///<summary>
	/// Thread main function. Threads will idle until work is available 
	/// and they have not received a sigterm from the main thread.
	///</summary>
	///<param name="ku_li_MY_ID_">The id of this thread within the thread pool.</param>
	void idle_thread(const std::size_t ku_li_MY_ID_)
	{
		auto b_run = true;

		while (b_run == true)
		{
			auto fn_job = get_work(ku_li_MY_ID_);

			try
			{
				fn_job();
			} // end try
			catch (const std::exception& e)
			{
				Guard_t guard(m_mtx_exception);
				std::cerr << "[Thread " << ku_li_MY_ID_ << "]: An exception occurred while executing a job." << std::endl;
				std::cerr << e.what() << std::endl;
			}
			catch (...) // catch any kind of exception and alert the user
			{
				Guard_t guard(m_mtx_exception);
				std::cerr << "[Thread " << ku_li_MY_ID_ << "]: An exception occurred while executing a job." << std::endl;
				m_q_exception.push(std::current_exception());
			} // end catch all

			switch (m_vect_signals.at(ku_li_MY_ID_))
			{
			case THREAD_SIGNALS::SIGTERM:
				b_run = false;
				break;
			default:
				break;
			} // end switch
		} // end while

		{
			Guard_t guard(m_mtx_signals);
			m_vect_signals.at(ku_li_MY_ID_) = THREAD_SIGNALS::TERMINATING;
		}
	} // end idle_thread


	///<summary>
	/// Removes and returns the next job from the queue and sets the calling thread's status. 
	/// If no jobs are queued, the job defaults to yield. 
	///</summary>
	///<param name="ku_li_MY_ID_">The id of the calling thread within the thread pool.</param>
	///<returns>A callable function object that the thread should execute.</returns>
	std::function<void(void)> get_work(const std::size_t ku_li_MY_ID_)
	{
		Guard_t guard(m_mtx_tasks);

		std::function<void(void)> job;

		if (m_q_tasks.empty() == false)
		{
			job = std::move(m_q_tasks.front());
			m_q_tasks.pop();

			Guard_t guard(m_mtx_signals);
			if (m_vect_signals[ku_li_MY_ID_] != THREAD_SIGNALS::SIGTERM)
			{
				m_vect_signals[ku_li_MY_ID_] = THREAD_SIGNALS::WORKING;
			} // end if
		} // end if
		else
		{
			job = std::this_thread::yield;

			Guard_t guard(m_mtx_signals);
			if (m_vect_signals[ku_li_MY_ID_] != THREAD_SIGNALS::SIGTERM)
			{
				m_vect_signals[ku_li_MY_ID_] = THREAD_SIGNALS::IDLE;
			} // end if
		} // end else

		return job;
	} // end method get_work


private:
	std::size_t mu_li_nthreads;                          //! the number of threads
	std::size_t mu_li_nrunning;                          //! the number of running threads
             
	std::vector<std::thread> m_vect_threads;             //! container storing thread objects
	std::vector<int>         m_vect_signals;             //! signal vector to communicate with threads
             
	mutable std::mutex m_mtx_tasks;                      //! mutex protecting the task queue
	mutable std::mutex m_mtx_signals;                    //! mutex protecting the signals vector
	mutable std::mutex m_mtx_exception;                  //! mutex protecting the exception queue

	std::queue<std::function<void(void)>> m_q_tasks;     //! queue storing tasks waiting for execution
	std::queue<std::exception_ptr>        m_q_exception; //! queue storing exceptions that occurred during execution of past jobs

}; // end class ThreadPool

#endif
