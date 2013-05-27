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
template<typename T>
class ServantFactoryCreator
{

public:

	ServantFactoryCreator()
	{}

	T* getServant()
	{
		return new T();
	}

protected:

	virtual ~ServantFactoryCreator(){}
};

//jw prototyp
template<typename T>
class ServantPrototypeCreator
{

public:

	ServantPrototypeCreator(T* pObj = 0)
		:pPrototype_(pObj)
	{}

	T* getServant()
	{
		return pPrototype_ ? new T(*pPrototype_) : 0;
	}

	T* GetPrototype()
	{
		return pPrototype_;
	}

	void SetPrototype(T* pObj)
	{ 
		pPrototype_ = pObj;
	}

private:

	T* pPrototype_;

protected:

	virtual ~ServantPrototypeCreator()
	{
		delete pPrototype_;
	}
};

//parametr: typ servanta i wytyczna jego tworzenia
//przyklad w pliku Example1.hpp
template<class Servant, template <class U> class ServantCreationPolicy>
class Proxy: public ServantCreationPolicy<Servant>
{
	mutable Logger log_;

protected:

	//obie skladowe musza byc sparametryzowane konkretnym servantem
	std::vector<Scheduler<Servant>* > schedulers_;
	ActivationQueue<Servant>* AQ_;

	Proxy(int numThreads=1):
		AQ_(new ActivationQueue<Servant>()),
		log_("PROXY",2)
	{
		log_<<"constructor"<<endl;
		for(int i=0;i<numThreads;++i)
		{
			//korzystamy z wytycznej do wygenerowania wskaznika do servanta
			Servant* serv = getServant();
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
		log_<<"destructor"<<endl;
		for_each( schedulers_.begin(), schedulers_.end(), stopScheduler);
		//TODO dodac kasowanie schedulerow
		delete AQ_;
	}

};

#endif