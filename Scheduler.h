#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "ActivationQueue.h"
#include "Command.hpp"
#include <boost/thread/mutex.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>


/**
* Scheduler responsible for managing method queueing and executing.
* @authors Michal Kosyl Marta Kuzak
* @version 1.0
*/

class Scheduler
{
	typedef boost::shared_ptr<Funktor> FunPtr;
public:
	Scheduler(void) {}
	~Scheduler(void) {}
	void enqueue(FunPtr f) {
		boost::mutex::scoped_lock lock(mutex_);
		queue_.push(f);
	}
private:
	void dequeue()  {
		while(!queue_.empty()){
			boost::mutex::scoped_lock lock(mutex_);
			queue_.pop()->execute();
		}
	}

	ActivationQueue queue_;
	mutable boost::mutex mutex_;
};


#endif
