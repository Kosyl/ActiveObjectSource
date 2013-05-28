#ifndef ACTIVATIONQUEUE_HPP
#define ACTIVATIONQUEUE_HPP

#include "MethodRequest.hpp"
#include <queue>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

/**
* A queue that is managed by Scheduler.
* @authors Michal Kosyl Marta Kuzak
* @version 1.0
*/

//sparametryzowalem ja, bedzie potrzebna informacja o konkretnym servancie
//i przechowuje zwykle wskazniki do funktorow, a nie sprytne
//mialem problem z shared_ptr do templatek, nie wiem czy czegos nie potrafilem czy faktycznie sie nie da
template<class Servant>
class ActivationQueue
{

private:
	/**
	* queue for client requests
	*/
	std::queue <Functor<Servant>* > queue_; 
	mutable boost::mutex mutex_;
	boost::condition_variable cond_;
	mutable Logger log_;
	volatile bool shouldIEnd_;
public:

	/*
	* Non-parameter constructor. 
	*/
	ActivationQueue(void): 
		shouldIEnd_(false),
		log_("AQ",4)
	{
		DLOG(log_<<"constructor"<<endl);
	}

	~ActivationQueue(void)
	{
		DLOG(log_<<"destructor"<<endl);
		while(!queue_.empty())
		{
			Functor<Servant>* tmp= queue_.front();
			queue_.pop();
			delete tmp;
		}
	}

	/**
	* pushes MethodRequest object to the queue
	* @param mr MethodRequest object to be pushed into queue
	*/
	void push(Functor<Servant>* f)
	{ 
		boost::mutex::scoped_lock lock(mutex_);
		DLOG(log_<<"push"<<endl);
		queue_.push(f);
		lock.unlock();
		cond_.notify_one();
	}

	/**
	* @brief popping MethodRequest from the queue
	* @return MethodRequest object that is popped
	*/
	Functor<Servant>* pop()
	{
		boost::mutex::scoped_lock lock(mutex_);
		DLOG(log_<<"pop"<<endl);
		while((!shouldIEnd_) && queue_.empty()) 
		{
			DLOG(log_<<"scheduler waiting for signal"<<endl);
			cond_.wait(lock);
		}
		if(shouldIEnd_)
		{
			DLOG(log_<<"returning null to scheduler"<<endl);
			return NULL;
		}
		DLOG(log_<<"returning functor to scheduler"<<endl);
		Functor<Servant>* tmp= queue_.front();
		queue_.pop();
		return tmp;
	}

	/**
	* @return whether the queue is empty
	*/
	bool empty() const 
	{
		boost::mutex::scoped_lock lock(mutex_);
		DLOG(log_<<"empty (" << queue_.empty() << ")" <<endl);
		return queue_.empty();
	}

	// to chyba wszystkie niezbedne metody, ale mozna dodac, zeby byla kolejka full-wypas
	Functor<Servant>* front()
	{
		boost::mutex::scoped_lock lock(mutex_);
		DLOG(log_<<"front"<<endl);
		return queue_.front();
	}

	/**
	* @return size of the queue 
	*/
	unsigned int size() const 
	{
		boost::mutex::scoped_lock lock(mutex_);
		DLOG(log_<<"size: " << queue_.size() <<endl);
		return queue_.size();
	}

	/**
	* setter of shouldIEnd
	*/
	void End() 
	{
		boost::mutex::scoped_lock lock(mutex_);
		DLOG(log_<<"End()"<<endl);
		shouldIEnd_=true;
		cond_.notify_all();
	}

};
//////////////////////////////////////////////////////////////////////////

#endif

