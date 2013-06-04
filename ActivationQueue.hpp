/**
* @file ActivationQueue.hpp
* @author Michal Kosyl
* @author Marta Kuzak
* @details Active Object implementation.
* @details ActivationQueue is a queue for client's requests. 
*/
#ifndef _ACTIVATIONQUEUE_HPP_
#define _ACTIVATIONQUEUE_HPP_

#include "MethodRequest.hpp"
#include <queue>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

namespace ActiveObject
{

	/**
	* ActivationQueue is created by Proxy that pushes Functors into it. Functors are dequeued by Schedulers.
	* @brief A thread-safe queue of Functor objects. 
	* @tparam Servant Type of Servant which executes method
	*/
	template<class Servant>
	class ActivationQueue
	{

	private:

		/**
		* queue for client requests (pointers to Functor objects)
		*/
		std::queue <Functor<Servant>* > queue_; 

		/**
		* Flag that indicates whether ActivationQueue is to be destroyed.
		* @see void End()
		*/
		volatile bool shouldIEnd_;

		/**
		* Mutex for synchronization
		*/
		mutable boost::mutex mutex_;

		/**
		* Makes thread wait when it tries to take Functor from the empy queue and notifies about new Functor pushed to the queue.
		*/
		boost::condition_variable cond_;

		/**
		* Condition variable used to notify the Schedulers to check again method guards
		*/
		boost::condition_variable refreshGuards_;

		/**
		* Number of methods in the queue, which are unable to execute because of their guards
		* @see Functor<Servant>* Pop()
		*/
		unsigned int guardedCount_;

		/**
		* periodically orders Schedulers to check the guards of all the queued MethodRequests
		* Useful, when guard() is waiting e.g. for memory availability, not some "queue-unblocking" MethodRequest
		*/
		boost::thread refreshGuardsThread_;

		/**
		* thread-safe logger
		*/
		DLOG(mutable Logger log_;)

	public:

		/**
		* Non-parameter constructor. 
		*/
		ActivationQueue(void): 
			shouldIEnd_(false),
			guardedCount_(0)
		{
			DLOG(log_.setName("AQ"));
			DLOG(log_.setColor(4));
			DLOG(log_<<"constructor"<<endl);
		}

		/**
		* Constructor launching separate guard-refreshing thread
		* @param refreshPeriod period of guard-check [ms]
		*/
		ActivationQueue(unsigned long refreshPeriod): 
			shouldIEnd_(false),
			guardedCount_(0)
		{
			if(refreshPeriod==0L)
			{
				throw NonPositivePeriodException();
			}

			DLOG(log_.setName("AQ"));
			DLOG(log_.setColor(4));
			DLOG(log_<<"constructor"<<endl);
			refreshGuardsThread_=boost::thread(boost::bind(&ActivationQueue::refreshFunction,this,refreshPeriod));
		}

		/**
		* Destructor
		* Cleans up its internal queue
		*/
		~ActivationQueue(void)
		{
			DLOG(log_<<"destructor"<<endl);
			while(!queue_.empty())
			{
				Functor<Servant>* tmp= queue_.front();
				queue_.pop();
				if (tmp)delete tmp;
			}
		}

		/**
		* Guard refresh function to be run in a separate thread
		* @param refreshPeriod period of guard-check [ms]
		*/
		void refreshFunction(unsigned long refreshPeriod)
		{
			while(!shouldIEnd_)
			{
				boost::this_thread::sleep(boost::posix_time::milliseconds(refreshPeriod));
				DLOG(log_<<"refreshing guards"<<endl);
				boost::mutex::scoped_lock lock(mutex_);

				guardedCount_=0;
				lock.unlock();
				refreshGuards_.notify_one();
			}
			DLOG(log_<<"koncze odswiezanie guards"<<endl);
		}


		/**
		* Pushes element into the queue and notifies one thread.
		* @brief pushes Functor into the queue.
		* @param f Functor pointer to a specific MethodRequest to be pushed into ActivationQueue
		*/
		void push(Functor<Servant>* f)
		{ 
			boost::mutex::scoped_lock lock(mutex_);

			queue_.push(f);
			guardedCount_=0;
			lock.unlock();
			cond_.notify_one();
			refreshGuards_.notify_all();
			DLOG(log_<<"push, rozmiar kolejki: " << queue_.size()<<endl);
		}

		/**
		* If the queue is empty it waits for any Functor pushed into the queue.
		* Also, checks whether the guard() conditions of each MethodRequest are met.
		* If none of the queued methods is executable, waits for refreshGuards notification
		* @brief pops Functor pointer from the queue.
		* @return pointer Functor object that is popped.
		*/
		Functor<Servant>* pop(boost::shared_ptr<Servant> servant)
		{
			boost::mutex::scoped_lock lock(mutex_);
			DLOG(log_<<"pop"<<endl);
			while((!shouldIEnd_) && queue_.empty()) 
			{
				DLOG(log_<<"scheduler waiting for signal"<<endl);
				cond_.wait(lock);
			}
			if(shouldIEnd_)//finishing the entire ActiveObject
			{
				DLOG(log_<<"returning null to scheduler"<<endl);
				return NULL;
			}

			Functor<Servant>* tmp=NULL;
			while(guardedCount_<queue_.size())//check for guards
			{
				DLOG(log_<<"sprawdzam guarda po raz " << guardedCount_ <<endl);
				tmp = queue_.front();
				queue_.pop();

				if(tmp->guard(servant)==true)
				{
					DLOG(log_<<"metoda zablokowana, szukam dalej..."<<endl);
					queue_.push(tmp);
					++guardedCount_;
				}
				else
				{
					DLOG(log_<<"metoda odblokowana!"<<endl);
					guardedCount_=0;
					break;
				}
			}

			DLOG(log_<<"wychodze, rozmiar kolejki:" << queue_.size() << ", ilosc przejrzanych: " << guardedCount_ << endl);
			if(guardedCount_>0 && guardedCount_>=queue_.size())//if all requests are guarded
			{
				DLOG(log_<<"wszystkie metody zablokowane, czekam na nowa..."<<endl);
				refreshGuards_.wait(lock);
				return NULL;
			}

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
		* Also, notifies the Schedulers to stop waiting.
		*/
		void End() 
		{
			boost::mutex::scoped_lock lock(mutex_);
			DLOG(log_<<"End()"<<endl);
			shouldIEnd_=true;
			cond_.notify_all();
			lock.unlock();
			refreshGuardsThread_.join();
		}

	};//ActivationQueue

}//ActiveObject

#endif

