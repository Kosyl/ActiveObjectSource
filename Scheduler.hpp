#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "ActivationQueue.hpp"
#include "Command.hpp"
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

class Scheduler
{
	typedef boost::shared_ptr<Functor> FunPtr;
public:
	Scheduler(void) {}
	/*
	* @param q pointer to queue
	* @patam s pointer to servant
	*/
	Scheduler(ActivationQueue* q, Servant* s): queue_(q), servant_(s) {
		thread_=boost::thread(boost::bind(&Scheduler::run,this));
	}


	~Scheduler(void) {}
	void enqueue(FunPtr f) {
		boost::mutex::scoped_lock lock(mutex_);
		queue_->push(f);
	}

	//void notify
	
	
private:
	void dequeue()  {
		while(!queue_->empty()){
			boost::mutex::scoped_lock lock(mutex_);
			FunPtr fun= queue_->pop();
			if(fun) fun->execute();
			else break;
		}
	}
	void run() { //nie wiem, czy to trafiana nazwa, chodzi o funkcje, ktora ma leciec przez caly cykl zycia Schedulera
		while(true){ //jakis tam warunek
			//while(!queue_->empty()){
				//boost::lock_guard<boost::mutex> guard(mutex_);// w dequeue sa locki 
				dequeue();
			//}
		}
	}

	bool stop() { //zatrzymywanie watku
		thread_.join();
		//jakas flaga na false?
		return true;
	}

	
	ActivationQueue* queue_;
	//Proxy* proxy_; //chyba nie
	Servant* servant_;
	boost::thread thread_;
	mutable boost::mutex mutex_;
};

//Scheduler::


#endif
