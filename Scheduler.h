#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "ActivationQueue.h"
#include "Command.hpp"
#include <boost/thread/mutex.hpp>
#include <boost/thread/detail/thread_group.hpp>
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
	typedef boost::shared_ptr<Funktor> FunPtr;
public:
	Scheduler(void) {}
	/**
	* @param n - number of threads
	*/
	Scheduler(int n) : n_(n), alive(true) {
		boost::lock_guard<boost::mutex> guard(mutex_);
        for(int i = 0; i < n_; ++i) {     
			std::auto_ptr<boost::thread> tmp_thread(new boost::thread(boost::bind(&Scheduler::run,this)));
            threads_.add_thread(tmp_thread.get());
			tmp_thread.release();
        }
		threads_.join_all();
    }

	~Scheduler(void) {
		alive= false;
		threads_.interrupt_all(); // czy to zgrabny sposob?
	}
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
	void run() { //nie wiem, czy to trafiana nazwa, chodzi o funkcje, ktora ma leciec przez caly cykl zycia Schedulera
		while(alive){
			while(!queue_.empty()){
				boost::lock_guard<boost::mutex> guard(mutex_);
				dequeue();
			}
		}
	}


	ActivationQueue queue_;
	mutable boost::mutex mutex_;
	boost::thread_group threads_;
	int n_;
	bool alive; //znowu ta sama uwaga co przy run() :D
};

//Scheduler::


#endif
