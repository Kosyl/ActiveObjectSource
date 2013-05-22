#ifndef ACTIVATIONQUEUE_HPP
#define ACTIVATIONQUEUE_HPP

#include "Command.hpp"
#include <queue>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

/**
* A queue that is managed by Scheduler.
* @authors Michal Kosyl Marta Kuzak
* @version 1.0
*/

class ActivationQueue
{
	typedef boost::shared_ptr<Funktor> FunPtr;
public:
	/*
	* Non-parameter constructor. 
	*/
	ActivationQueue(void) {}
	~ActivationQueue(void) {}
	/**
	* pushes MethodRequest object to the queue
	* @param mr MethodRequest object to be pushed into queue
	*/
	void push(FunPtr f) { 
		boost::mutex::scoped_lock lock(mutex_);
		queue_.push(f);
	}

	/**
	* popping MethodRequest from the queue
	* @return MethodRequest object that is popped
	*/
	FunPtr pop() {
		boost::mutex::scoped_lock lock(mutex_);
		while(queue_.empty()) {
			cond_.wait(lock);
		}
		FunPtr tmp= queue_.front();
		queue_.pop();
		return tmp;
	}

	/**
	* @return whether the queue is empty
	*/
	bool empty() const {
        boost::mutex::scoped_lock lock(mutex_);
        return queue_.empty();
    }
	// to chyba wszystkie niezbedne metody, ale mozna dodac, zeby byla kolejka full-wypas
	FunPtr front()  {
        boost::mutex::scoped_lock lock(mutex_);
        return queue_.front();
    }
	/**
	* @return size of the queue 
	*/
	unsigned int size() {
		return queue_.size();
	}

private:
	/**
	* queue for client requests
	*/
	std::queue <FunPtr> queue_; 
	mutable boost::mutex mutex_;
	boost::condition_variable cond_;
};
//////////////////////////////////////////////////////////////////////////

#endif

