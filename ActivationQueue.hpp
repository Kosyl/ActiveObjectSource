#ifndef ACTIVATIONQUEUE_HPP
#define ACTIVATIONQUEUE_HPP

#include "MethodRequest.hpp"
#include <queue>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

/**
* ActivationQueue is created by Proxy that pushes Functors into it. Functors are dequeued by Schedulers.
* @brief A thread-safe queue of Functor objects. 
* @tparam Servant Type of Servant which executes method
*/

//sparametryzowalem ja, bedzie potrzebna informacja o konkretnym servancie
//i przechowuje zwykle wskazniki do funktorow, a nie sprytne
//mialem problem z shared_ptr do templatek, nie wiem czy czegos nie potrafilem czy faktycznie sie nie da
template<class Servant>
class ActivationQueue
{

private:
	/**
	* queue for client requests (pointers to Functor objects)
	*/
	std::queue <Functor<Servant>* > queue_; 
	// - opisywanie mutexow itp chyba by by³o przesada :D
	mutable boost::mutex mutex_;
	boost::condition_variable cond_;
	mutable Logger log_;
	/**
	* Flag that indicates whether ActivationQueue is to be destroyed.
	* @see void End()
	*/
	volatile bool shouldIEnd_;
public:

	/**
	* Non-parameter constructor. 
	*/
	ActivationQueue(void): 
		shouldIEnd_(false),
		log_("AQ",4)
	{
		DLOG(log_<<"constructor"<<endl);
	}
	/**
	* Destructor
	*/
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
	* After that it notifies one thread about the event.
	* @brief pushes Functor into the queue.
	* @param mr Functor to be pushed into ActivationQueue
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
	* If the queue is empty it waits for any Functor pushed into the queue.
	* @brief pops Functor pointer from the queue.
	* @return pointer Functor object that is popped.
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
	* Test whether the queue is empty.
	* @return whether the queue is empty.
	*/
	bool empty() const 
	{
		boost::mutex::scoped_lock lock(mutex_);
		DLOG(log_<<"empty (" << queue_.empty() << ")" <<endl);
		return queue_.empty();
	}

	/**
	* Access next element.
	* @return pointer to the next element in the queue.
	*/
	Functor<Servant>* front()
	{
		boost::mutex::scoped_lock lock(mutex_);
		DLOG(log_<<"front"<<endl);
		return queue_.front();
	}

	/**
	* Returns size of the queue.
	* @return number of elements in the queue. 
	*/
	unsigned int size() const 
	{
		boost::mutex::scoped_lock lock(mutex_);
		DLOG(log_<<"size: " << queue_.size() <<endl);
		return queue_.size();
	}

	/**
	* Makes ActivationQueue stop waiting for next Functor in the queue.
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

