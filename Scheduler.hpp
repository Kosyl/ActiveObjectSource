#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "ActivationQueue.hpp"
#include "MethodRequest.hpp"
#include "Proxy.hpp"
#include "SimpleLog.hpp"
#include <boost/thread/mutex.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

//W BUDOWIE//
/**
* Scheduler responsible for managing method queueing and executing.
* @authors Michal Kosyl Marta Kuzak
* @version 1.0
*/
template<class Servant>
class Scheduler
{
private:
	ActivationQueue<Servant>* queue_;
	Servant* servant_;
	boost::thread thread_;
	mutable boost::mutex mutex_;
	volatile bool shouldIEnd_;
	mutable Logger log_;
public:

	/*
	* @param q pointer to queue
	* @param s pointer to servant
	*/
	Scheduler(ActivationQueue<Servant>* q, Servant* s):
		queue_(q),
		servant_(s),
		shouldIEnd_(false),
		log_("Scheduler",3),
		thread_(boost::thread(boost::bind(&Scheduler::run,this)))
		
	{
		DLOG(log_ << "constructor" << endl);
	}

	~Scheduler(void) 
	{
		DLOG(log_ << "destructor" << endl);
		delete servant_;
	}

	bool stop() 
	{ 
		shouldIEnd_=true;
		DLOG(log_ << "stop() - joining" << endl);
		thread_.join();
		DLOG(log_ << "stop() - joined" << endl);
		return true;
	}

private:

	void dequeue()  
	{
		boost::mutex::scoped_lock lock(mutex_);
		DLOG(log_ << "dequeue" << endl);

		Functor<Servant>* fun= queue_->pop();
		if(shouldIEnd_ || fun==NULL)
		{
			DLOG(log_<<"breaking dequeue"<<endl);
			return;
		}

		if(fun->isReady())
		{
			DLOG(log_ << "invoking request" << endl);
			//mowimy servantowi schedulera, zeby wskazywal na ten sam content co zadanie wyjete z kolejki
			servant_->setFutureContent(fun->getFutureContent());

			//tu wywolanie wlasciej funkcji; przechwytywanie wyjatkow jest w MethodRequest
			fun->execute(servant_);
		}
		//else fun->getFutureContent()->setException(boost::copy_exception(new NullCommandException));
		delete fun;
	}

	void run() 
	{ 
		DLOG(log_ << "run" <<endl);
		while(!shouldIEnd_)
		{ 
			dequeue();
		}
		DLOG(log_ << "thread finishing..." <<endl);
	}	
};

//Scheduler::

#endif
