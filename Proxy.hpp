/**
* @file Proxy.hpp
* @author Michal Kosyl
* @author Marta Kuzak
* @details Active Object implementation.
* @details Proxy enables client to invoke its method on Active Object. When client invokes method on Proxy, it is pushed into ActivationQueue as Functor.
* Then it waits for Scheduler to take it from the queue and make Servant execute.
* @details This file also provides with some variants of creating Servant- Singleton, Prototype, Factory.
*/
#ifndef _PROXY_
#define _PROXY_

#include "Future.hpp"
#include "MethodRequest.hpp"
#include "FutureContentCreator.hpp"
#include "Scheduler.hpp"
#include "ActivationQueue.hpp"
#include <boost/function.hpp>
#include <vector>

//Wybacz zmianê nazwy wyj¹tku. 
/**
* thrown when resfreshPeriod parameter is non-positive.
* refreshPeriod indicates how often result of guard method is checked. 
*/
class NonPositivePeriodException: public exception, public boost::exception
{
public:
    NonPositivePeriodException(){}

    virtual const char* what()
    {
		return "Refresh period has to be greater than 0!";
    }
};
namespace ActiveObject
{
    using namespace std;
    
    //daje metode getServant ktora dziala jak fabryka
    /**
	 * ServantFactoryCreator allows to create every time different Servant.
     * @brief Servant Factory.
     * @tparam T Type of servant to create.
	 * @see ServantPrototypeCreator
	 * @see ServantSingletonCreator
     */
    template<typename T>
    class ServantFactoryCreator
    {
	
    public:
		/**
		* @brief Constructor
		*/
		ServantFactoryCreator()
		{}
		/**
		* @brief Returns pointer to the new created Servant.
		* @return pointer to the created Servant.
		*/
		boost::shared_ptr<T> getServant()
		{
			return boost::shared_ptr<T>(new T);
		}
	
		/**
		* @brief Destructor
		*/
		virtual ~ServantFactoryCreator(){}
    };
    
    //jw prototyp
    /**
	 * In this variant Servants are created by cloning prototypical instance of Servant.
     * @brief Servant Prototype
     * @tparam T Type of servant prototype.
	 * @see ServantFactoryCreator
	 * @see ServantSingletonCreator
     */
    template<typename T>
    class ServantPrototypeCreator
    {
	
    public:
		/**
		* Sets prototype. 
		* @brief Constructor.
		* @param pObj Pointer to Servant which is to be prototype
		*/
		ServantPrototypeCreator(T* pObj = 0)
		:pPrototype_(pObj)
		{}
		/**
		* @return Pointer to created Servant
		*/
		boost::shared_ptr<T> getServant()
		{
			return pPrototype_ ? pPrototype_ : 0;
		}
		/**
		* @return Pointer to prototype.
		*/
		boost::shared_ptr<T> GetPrototype()
		{
			return pPrototype_;
		}
		/**
		* Sets prototype with given value
		* @param pObj Pointer to Servant that is to be prototype.
		*/
		void SetPrototype(T* pObj)
		{ 
			pPrototype_ = pObj;
		}
	
	private:
		/**
		* Pointer to prototype
		*/
		boost::shared_ptr<T> pPrototype_;
		/**
		* Destructor.
		*/
		virtual ~ServantPrototypeCreator()
		{
	    }
    };
    
    //jw singleton
	/**
	 * In this solution only one Servant can exists. 
     * @brief Servant Singleton
     * @tparam T Type of servant to create.
	 * @see ServantFactoryCreator
	 * @see ServantPrototypeCreator
     */
    template<typename T>
    class ServantSingletonCreator
    {
	
    public:
		/** 
		* Constructs ServantSingletonCreator and creates new (and the only) instance of Servant.
		*/
		ServantSingletonCreator()
		:pInstance_(new T)
		{}
		/**
		* @return Pointer to instance of Servant.
		*/
		boost::shared_ptr<T> getServant()
		{
			return pInstance_;
		}
		/**
		* Destruct ServantSingletonCreator
		*/
		virtual ~ServantSingletonCreator(){}
    private:
		/**
		* Pointer to object of Servant.
		*/
		boost::shared_ptr<T> pInstance_;
	
    };
    
    //parametr: typ servanta i wytyczna jego tworzenia
    //przyklad w pliku Example1.hpp
	/**
	* @brief Proxy provides client with ability to invoke its method on Active Object. 
	* @details Using Future returned by Proxy client can 
	* check progress, state and- after the method execution is finished- get result of the method.
	* @tparam Servant Type of Servant
	* @tparam ServantCreationPolicy Type of Policy //?? KARDAMON- jak to po angielsku jest?
	* @tparam U Param of Policy
	* @see ServantFactoryCreator
	* @see ServantPrototypeCreator
	* @see ServantSingletonCreator 
	*/
    template<class Servant, template <class U> class ServantCreationPolicy>
    class Proxy
    {
    protected:
		/**
		* Vector of Pointers to Schedulers- parametrized by given Servant type. 
		*/
		std::vector<Scheduler<Servant>* > schedulers_;
		/**
		* Pointer to queue of client requests.
		*/
		ActivationQueue<Servant>* AQ_;
		/**
		* Thread-safe logger
		*/
		DLOG(mutable Logger log_);
		/**
		* Policy of Servant Creation.
		*/
		ServantCreationPolicy<Servant> servantCreator_;
		/**
		* @brief Constructs Proxy with given number of threads.
		* @details In every thread run one Scheduler.
		* @param numThreads- number of threads and Schedulers as well.
		*/
		Proxy(unsigned int numThreads=1):
		AQ_(new ActivationQueue<Servant>())
		{
			DLOG(log_.setName("Proxy"));
			DLOG(log_.setColor(2));
			DLOG(log_<<"constructor"<<endl);
			for(unsigned int i=0;i<numThreads;++i)
			{
				//korzystamy z wytycznej do wygenerowania wskaznika do servanta
				boost::shared_ptr<Servant> serv = servantCreator_.getServant();
				schedulers_.push_back(new Scheduler<Servant>(AQ_,serv));
				}
		}
		/**
		* @brief Constructs Proxy with given number of threads and refreshPeriod.
		* @details In every thread run one Scheduler.
		* @param numThreads- number of threads and Schedulers as well.
		* @param refreshPeriod period of guard-check [ms]
		* @throw NonPositivePeriodException when refreshPeriod is not positive.
		*/
		//KARDAMON: czy to mo¿liwe, skoro mamy unsigned longa?
		Proxy(unsigned int numThreads, unsigned long refreshPeriod)
		{
			DLOG(log_.setName("Proxy"));
			DLOG(log_.setColor(2));
			DLOG(log_<<"constructor"<<endl);
			if(refreshPeriod>0)
			{
				AQ_=new ActivationQueue<Servant>(refreshPeriod);
			}
			else
				throw NonPositivePeriodException();
	    
			for(unsigned int i=0;i<numThreads;++i)
			{
				//korzystamy z wytycznej do wygenerowania wskaznika do servanta
				boost::shared_ptr<Servant> serv = servantCreator_.getServant();
				schedulers_.push_back(new Scheduler<Servant>(AQ_,serv));
			}
		}
		/**
		* Stops the ActivationQueue and waits for all Schedulers join. After that it deletes Schedulers, then ActivatioQueue and in the end itself.
		*/
		virtual ~Proxy()
		{
			DLOG(log_<<"destructor"<<endl);
	    
			for_each( schedulers_.begin(), schedulers_.end(), stopScheduler);
			AQ_->End();
			for_each( schedulers_.begin(), schedulers_.end(), joinScheduler);
	    
			DLOG(log_<<"deleting schedulers"<<endl);
			for(unsigned int i=0;i<schedulers_.size();++i)
			{
				delete schedulers_[i];
			}
			delete AQ_;
		}
		/**
		* Stops all Schedulers one by one.
		*/
		struct stop 
		{
			/** 
			* @param i Pointer to Scheduler which is to stop
			*/
			void operator() (Scheduler<Servant>* i) 
			{
				i->stopOrder();
			}
		} stopScheduler;
		/**
		* Joins all Schedulers one by one.
		*/
		struct joinSchedul 
		{
			/** 
			* @param i Pointer to Scheduler which is to stop
			*/
			void operator() (Scheduler<Servant>* i) 
			{
				i->joinThread();
			}
		} joinScheduler;

		/**
		* @brief Creates Future, its FutureContent and MethodRequest. MethodRequest is pushed into the queue. 
		* @param command boost::function object which keeps address to clients method invokation. Finally it returns created Future.
		* @tparam T Type of result value of invoked method.
		* @return Future which allows client to get info about state, progress and return value of client request.
		*/
		template<typename T>
		Future<T> enqueue(boost::function<T(Servant*)> command)
		{
			DLOG(log_ << "enqueue()" << endl);
			boost::shared_ptr<FutureContent> pContent(new FutureContent());
			Future<T> fut(pContent);
			MethodRequest<T,Servant>* request = new MethodRequest<T,Servant>(command,pContent);
			AQ_->push(request);
			return fut;
		}
		/**
		* @brief Creates Future, its FutureContent and MethodRequest. MethodRequest is pushed into the queue. 
		* @param command boost::function object which keeps address to clients method invokation. Finally it returns created Future.
		* @param guard guard function that indicates whether the method should not be executed. 
		* @tparam T Type of result value of invoked method.
		* @return Future which allows client to get info about state, progress and return value of client request.
		*/
		template<typename T>
		Future<T> enqueue(boost::function<T(Servant*)> command, boost::function<bool(Servant*)> guard)
		{
			DLOG(log_ << "enqueue()" << endl);
			boost::shared_ptr<FutureContent> pContent(new FutureContent());
			Future<T> fut(pContent);
			MethodRequest<T,Servant>* request = new MethodRequest<T,Servant>(command,pContent, guard);
			AQ_->push(request);
			return fut;
		}
    };
    
}//ActiveObject
#endif
