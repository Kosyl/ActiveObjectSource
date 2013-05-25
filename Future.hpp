#ifndef _FUTURE_
#define _FUTURE_

#include <boost\smart_ptr\shared_ptr.hpp>
#include <boost\thread.hpp>
#include "SimpleLog.hpp"
#include <boost\noncopyable.hpp>
#include <boost\signal.hpp>
#include <boost\any.hpp>
#include <vector>
#include "FutureContent.hpp"

using namespace std;

template<typename T>
class Future
{
private:
	boost::shared_ptr<FutureContent> pFutureContent_;
	boost::function<void(double)> progressSlot_;
	boost::signals::connection progressConnection_;
	Logger log_;

public:
	Future(boost::shared_ptr<FutureContent> target):
		pFutureContent_(target),
		log_("FUTURE")
	{	
		log_ << "constructor" << endl;
	}

	//todo: konwersja miedzy future

	Future(Future& rhs):
		pFutureContent_(rhs.pFutureContent_),
		log_("FUTURE")
	{
		log_ << "c constructor" << endl;
		if(progressSlot_)pFutureContent_->dettachProgressObserver(progressConnection_); //odczepienie sie od starego sygnalu
		setFunction(rhs.progressSlot_);
	}
/*
*
*/
	Future& operator=(const Future& rhs)
	{
		log_ << "= operator" << endl;
		pFutureContent_= rhs.pFutureContent_;
		if(progressSlot_)pFutureContent_->dettachProgressObserver(progressConnection_); //odczepienie sie od starego sygnalu
		setFunction(rhs.progressSlot_);
		return *this;
	}

	template<typename FuncType>
	void setFunction(FuncType fun)
	{
		log_ << "setFunction" << endl;
		progressSlot_=fun;
		if(progressSlot_)pFutureContent_->attachProgressObserver(progressSlot_);
	}

	T getValue()
	{
		if(!pFutureContent_)throw RequestCancelledException();
		log_ << "getValue (" << (boost::any_cast<T>(pFutureContent_->getValue())) << ")" << endl;
		return boost::any_cast<T>(pFutureContent_->getValue());
	}

	double getProgress()
	{
		if(!pFutureContent_)throw RequestCancelledException();
		log_ << "getProgress (" << pFutureContent_->getProgress() << ")" << endl;
		return pFutureContent_->getProgress();
	}

	exception getException()
	{
		if(!pFutureContent_)throw RequestCancelledException();
		log_ << "getException (" << (pFutureContent_->getException().what()) << ")" << endl;
		return pFutureContent_->getException();
	}

	bool hasException()
	{
		if(!pFutureContent_)throw RequestCancelledException();
		log_ << "hasException (" << pFutureContent_->hasException() << ")" << endl;
		return pFutureContent_->hasException();
	}

	bool isDone()
	{
		if(!pFutureContent_)throw RequestCancelledException();
		log_ << "isDone (" << pFutureContent_->isDone() << ")" << endl;
		return pFutureContent_->isDone();
	}

	void cancelRequest()
	{
		log_ << "cancelRequest" << endl;
		progressConnection_.disconnect();
		if(!pFutureContent_)pFutureContent_.reset();
		pFutureContent_=NULL;
	}

	~Future()
	{
		log_ << "~Future()" << endl;
	}

};

#endif