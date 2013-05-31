#ifndef _PROXY_
#define _PROXY_

#include "Future.hpp"
#include "MethodRequest.hpp"
#include "FutureContentCreator.hpp"
#include "Scheduler.hpp"
#include "ActivationQueue.hpp"
#include <boost\function.hpp>
#include <vector>

using namespace std;

//daje metode getServant ktora dziala jak fabryka
/**
* @brief Servant Factory.
* @tparam T Type of servant to create.
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
	* @brief Returns pointer to the created Servant.
	* @return pointer to the created Servant.
	*/
	boost::shared_ptr<T> getServant()
	{
		return boost::shared_ptr<T>(new T);
	}

protected:
	/**
	* @brief Destructor
	*/
	virtual ~ServantFactoryCreator(){}
};

//jw prototyp
/**
* @brief Servant Prototype
* @tparam T Type of servant to create.
*/
template<typename T>
class ServantPrototypeCreator
{

public:
	/**
	* @brief Constructor
	*/
	ServantPrototypeCreator(T* pObj = 0)
		:pPrototype_(pObj)
	{}

	boost::shared_ptr<T> getServant()
	{
		return pPrototype_ ? pPrototype_ : 0;
	}

	boost::shared_ptr<T> GetPrototype()
	{
		return pPrototype_;
	}

	void SetPrototype(T* pObj)
	{ 
		pPrototype_ = pObj;
	}

private:

	boost::shared_ptr<T> pPrototype_;

protected:

	virtual ~ServantPrototypeCreator()
	{
		
	}
};

//jw singleton
template<typename T>
class ServantSingletonCreator
{

public:

	ServantSingletonCreator()
		:pInstance_(new T)
	{}

	boost::shared_ptr<T> getServant()
	{
		return pInstance_;
	}

private:

	boost::shared_ptr<T> pInstance_;

protected:

	virtual ~ServantSingletonCreator(){}
};

//parametr: typ servanta i wytyczna jego tworzenia
//przyklad w pliku Example1.hpp
template<class Servant, template <class U> class ServantCreationPolicy>
class Proxy: public ServantCreationPolicy<Servant>
{
protected:
	mutable Logger log_;

	//obie skladowe musza byc sparametryzowane konkretnym servantem
	std::vector<Scheduler<Servant>* > schedulers_;
	ActivationQueue<Servant>* AQ_;

	Proxy(int numThreads=1):
		AQ_(new ActivationQueue<Servant>()),
		log_("Proxy",2)
	{
		DLOG(log_<<"constructor"<<endl);
		for(int i=0;i<numThreads;++i)
		{
			//korzystamy z wytycznej do wygenerowania wskaznika do servanta
			boost::shared_ptr<Servant> serv = getServant();
			schedulers_.push_back(new Scheduler<Servant>(AQ_,serv));
			//i robimy schedulera
			
		}
	}

	struct stop 
	{
		void operator() (Scheduler<Servant>* i) {
			i->stop();
		}
	} stopScheduler;

	virtual ~Proxy()
	{
		DLOG(log_<<"destructor"<<endl);
		AQ_->End();

		for_each( schedulers_.begin(), schedulers_.end(), stopScheduler);
		DLOG(log_<<"deleting schedulers"<<endl);
		for(unsigned int i=0;i<schedulers_.size();++i)
		{
			delete schedulers_[i];
		}

		delete AQ_;
	}

	template<typename T>
	Future<T> enqueue(boost::function<T(Servant*)> command)
	{
		DLOG(log_ << "enqueue() - content creation" << endl);
		boost::shared_ptr<FutureContent> pContent(new FutureContent());
		DLOG(log_ << "enqueue() - future creation" << endl);
		Future<T> fut(pContent);
		DLOG(log_ << "enqueue() - request creation" << endl);
		MethodRequest<T,Servant>* request = new MethodRequest<T,Servant>(command,pContent);
		Functor<Servant>* functor = request;
		DLOG(log_ << "enqueue() - pushing into AQ" << endl);
		AQ_->push(functor);
		DLOG(log_ << "enqueue() - returning future" << endl);
		return fut;
	}

};

#endif