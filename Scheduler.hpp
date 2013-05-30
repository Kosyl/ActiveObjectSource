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

/**
* @brief Dequeues client request from ActivationQueue and makes Servant to execute them.
* @tparam Servant Type of servant that executes methods. 
*/
template<class Servant>
class Scheduler
{
private:
	/**
	* Pointer to the ActivationQueue Scheduler looks at.
	*/
	ActivationQueue<Servant>* queue_;
	/**
	* Pointer to servant which executes method.
	*/
	boost::shared_ptr<Servant> servant_;
	/**
	* Thread of Scheduler.
	*/
	boost::thread thread_;
	/**
	*
	*/
	mutable boost::mutex mutex_;
	/**
	* Flag that indicates whether ActivationQueue is to be destroyed.
	* @see bool stop()
	*/
	volatile bool shouldIEnd_;
	/**
	* Logger
	*/
	mutable Logger log_;
public:

	/**
	* Constructs Scheduler and starts its thread.
	* @brief Constructor
	* @param q pointer to ActivationQueue
	* @param s pointer to Servant
	*/
	Scheduler(ActivationQueue<Servant>* q, boost::shared_ptr<Servant> s):
		queue_(q),
		servant_(s),
		shouldIEnd_(false),
		log_("Scheduler",3),
		thread_(boost::thread(boost::bind(&Scheduler::run,this)))
		
	{
		DLOG(log_ << "constructor" << endl);
	}
	/**
	* @brief Destructor.
	*/
	~Scheduler(void) 
	{
		DLOG(log_ << "destructor" << endl);
	}
	/**
	* Sets shouldIEnd to true and waits for its thread end.
	* @return true :)
	*/
	bool stop()  //yy czemu tu by³ bool?
	{ 
		shouldIEnd_=true;
		DLOG(log_ << "stop() - joining" << endl);
		thread_.join();
		DLOG(log_ << "stop() - joined" << endl);
		return true;
	}

private:
	/**
	* It is broken when request is NULL or shouldIEnd is set to true.
	* @brief Dequeues method request from ActivationQueue and makes Servant execute it.
	*/
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
	/**
	* As long as Scheduler exists it dequeues requests from ActivationQueue.
	* @brief method of the Scheduler thread.
	*/
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


#endif
