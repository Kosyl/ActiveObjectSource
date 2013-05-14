#ifndef _PROMISE_
#define _PROMISE_

#include "Future.hpp"
#include <boost\shared_ptr.hpp>
#include "SimpleLog.hpp"

template<typename T>
class Promise
{
	typedef boost::shared_ptr<Future<T>> PFuture;
	
private:
	Logger log;
	PFuture future_;

public:
	Promise():
		log("PROMISE"),
		future_(new Future<T>())
	{}

	PFuture getFuture()
	{
		log << "Get Future" << "\n";
		PFuture res = future_;
		return res;
	}

	void setProgress(const double& progress)
	{
		log << "Set progress: " << progress << endl;
		future_->setProgress(progress);  //literowka :P
	}

	void setValue(const T& val)
	{
		log << "SetValue: " << val << endl;
		future_->setValue(val);
	}

	bool isHalt()
	{
		return future_->isHalt();
	}
};

#endif