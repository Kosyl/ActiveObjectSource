#ifndef _PROXY_
#define _PROXY_

#include "Future.hpp"
#include "Command.hpp"
#include "Promise.hpp"
#include "Servant.hpp"
#include <boost\function.hpp>
#include <vector>

using namespace std;

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
};

template<typename T>
class prototypeServantCreator
{
public:
	prototypeServantCreator(T* pObj = 0)
		:pPrototype_(pObj)
	{}

	T* getServant()
	{
		return pPrototype_ ? pPrototype_->Clone() : 0;
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
};

template<class ServantCreationPolicy>
class Proxy: public ServantCreationPolicy
{
private:
	std::vector<Scheduler> schedulers_;

protected:
	Proxy(int numThreads=1)
	{
		for(int i=0;i<numThreads;++i)
		{
			Servant* serv = getServant();/*
			Scheduler scheduler(
			schedulers_.push_back(*/

		}
	}

	template<typename T>
	boost::shared_ptr<Future<T>> quickSchedule(boost::function<T()> fun)
	{
		Promise<T>* prom = new Promise<T>();
		boost::shared_ptr<Future<T>> fut = prom->getFuture();
		Funktor* funkt = new MethodRequest<T>(fun,prom);
		funktors_.push_back(funkt);
		return fut;
	}

	template<typename T>
	boost::shared_ptr<Future<T>> quickSchedule(Command<T>* context, boost::function<T()> fun)
	{
		boost::shared_pointer<Future<T>> fut = context->promise_->getFuture();
	}

};

#endif