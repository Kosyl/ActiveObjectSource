#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "ActivationQueue.hpp"
#include "MethodRequest.hpp"
#include "Proxy.hpp"
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
		thread_(boost::thread(boost::bind(&Scheduler::run,this))),
		log_("Scheduler",3)
	{
		log_ << "constructor" << endl;
	}

	~Scheduler(void) 
	{
		log_ << "destructor" << endl;
	}

	//void notify do przestania pracy
	bool stop() 
	{ 
		//zatrzymywanie watku
		log_ << "stop" << endl;
		shouldIEnd_=true;
		queue_->setShouldIEnd(true);
		thread_.join();
		//- jakas flaga na false?
		//- prosze uprzejmie
		return true;
	}

private:

	void dequeue()  
	{
		boost::mutex::scoped_lock lock(mutex_);
		log_ << "dequeue" << endl;
		//wskaznik na Functor
		//ale metody wirtualne wyceluja w dobra konkretna realizacje
		//tu byl jeszcze lock na queue.isEmpty, ale to jest sprawdzane w popie
		Functor<Servant>* fun= queue_->pop();
		if(shouldIEnd_) return;

		if(fun->isReady())
		{
			//mowimy servantowi schedulera, zeby wskazywal na ten sam content co zadanie wyjete z kolejki
			servant_->setFutureContent(fun->getFutureContent());

			//tu wywolanie wlasciej funkcji; przechwytywanie wyjatkow jest w MethodRequest
			fun->execute(servant_);
		}
		//else fun->getFutureContent()->setException(boost::copy_exception(new NullCommandException));
	}

	void run() 
	{ 
		//- nie wiem, czy to trafiana nazwa, chodzi o funkcje, ktora ma leciec przez caly cykl zycia Schedulera
		//- wyborna
		log_ << "run" <<endl;
		while(!shouldIEnd_)
		{ 
			// w dequeue sa locki 
			dequeue();
		}
	}	
	
};

//Scheduler::

#endif
