/**
* @file Scheduler.hpp
* @author Michal Kosyl
* @author Marta Kuzak
* @details Active Object implementation.
* @details Scheduler's task is to take Functors from the ActivationQueue and make Servants execute them. It runs in different thread than its client.
*/
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

namespace ActiveObject
{
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
		*
		*/
		mutable boost::mutex mutex_;
		/**
		* Flag that indicates whether ActivationQueue is to be destroyed.
		* @see void stopOrder()
		*/
		volatile bool shouldIEnd_;
		/**
		* Logger
		*/
		DLOG(mutable Logger log_;)
		/**
		* Thread of Scheduler.
		*/
		boost::thread thread_;
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
			shouldIEnd_(false)
		{
			DLOG(log_.setName("Scheduler"));
			DLOG(log_.setColor(3));
			DLOG(log_ << "constructor" << endl);
			thread_=boost::thread(boost::bind(&Scheduler::run,this));
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
		*/
		void stopOrder() 
		{ 
			DLOG(log_ << "stopOrder()" << endl);
			shouldIEnd_=true;
		}

		void joinThread()
		{
			DLOG(log_ << "stop() - joining" << endl);
			thread_.join();
			DLOG(log_ << "stop() - joined" << endl);
		}


	private:
		/**
		* It is broken when request is NULL or shouldIEnd is set to true.
		* @brief Dequeues method request from ActivationQueue and makes Servant execute it.
		*/
		void dequeue()  
		{
			boost::mutex::scoped_lock lock(mutex_);
			//DLOG(log_ << "dequeue" << endl);

			Functor<Servant>* fun= queue_->pop(servant_);
			if(shouldIEnd_ || fun==NULL)
			{
				//DLOG(log_<<"breaking dequeue"<<endl);
				return;
			}

			if(fun->isReady())
			{
				//DLOG(log_ << "invoking request" << endl);
				//mowimy servantowi schedulera, zeby wskazywal na ten sam content co zadanie wyjete z kolejki
				servant_->setFutureContent(fun->getFutureContent());
				fun->getFutureContent()->setState(INPROGRESS);
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

}//ActiveObject
#endif
