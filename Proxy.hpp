#ifndef _PROXY_
#define _PROXY_

#include "Future.hpp"
#include "Command.hpp"
#include "Promise.hpp"
#include <boost\function.hpp>
#include <vector>

using namespace std;

class Proxy
{
private:
	std::vector<Funktor*> funktors_;
public:
	Proxy(){}
	
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

	void quickExecute()
	{
		for(unsigned int i=0;i<funktors_.size();++i)
		{
			funktors_[i]->execute();
		}

	}

};

#endif