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

//todo: konwersja miedzy future
//i miedzy roznymi typami future, czyli troche templatek
template<typename T>
class Future
{

private:

	boost::shared_ptr<FutureContent> pFutureContent_;
	boost::function<void(double)> progressSlot_;
	boost::signals::connection progressConnection_;

	mutable Logger log_;

	void dummyCallback(double x)
	{}

public:

	Future(boost::shared_ptr<FutureContent> target):
		pFutureContent_(target),
		log_("Future",7),
		progressSlot_(boost::bind(&Future<T>::dummyCallback,this,_1))
	{	
		DLOG(log_ << "constructor" << endl);
		progressConnection_=pFutureContent_->attachProgressObserver(progressSlot_);
	}

	Future(const Future& rhs):
		pFutureContent_(rhs.pFutureContent_),
		log_("Future",7)
	{
		DLOG(log_ << "c constructor" << endl);

		setFunction(rhs.progressSlot_);
		progressConnection_=pFutureContent_->attachProgressObserver(progressSlot_);
		pFutureContent_->cancel(rhs.progressConnection_); //odczepienie sie od starego sygnalu
	}

	Future<T>& operator=(const Future& rhs)
	{
		DLOG(log_ << "= operator" << endl);
		if(&rhs==this) return *this;
		Future<T> res(*this);
		return res;
	}

	template<typename FuncType>
	void setFunction(FuncType fun)
	{
		DLOG(log_ << "setFunction" << endl);
		progressSlot_=fun;
	}

	T getValue() const
	{
		if(!pFutureContent_)
			throw RequestCancelledException();
		DLOG(log_ << "getValue ()" << endl);
		return boost::any_cast<T>(pFutureContent_->getValue());
	}

	operator T()
	{
		return getValue();
	}

	double getProgress() const
	{
		if(!pFutureContent_)
			throw RequestCancelledException();
		DLOG(log_ << "getProgress (" << pFutureContent_->getProgress() << ")" << endl);
		return pFutureContent_->getProgress();
	}

	exception getException() const
	{
		if(!pFutureContent_)
			throw RequestCancelledException();
		DLOG(log_ << "getException (" << (pFutureContent_->getException().what()) << ")" << endl);
		return pFutureContent_->getException();
	}

	bool hasException() const
	{
		if(!pFutureContent_)
			throw RequestCancelledException();
		DLOG(log_ << "hasException (" << pFutureContent_->hasException() << ")" << endl);
		return pFutureContent_->hasException();
	}

	bool isDone() const
	{
		if(!pFutureContent_)
			throw RequestCancelledException();
		DLOG(log_ << "isDone (" << pFutureContent_->isDone() << ")" << endl);
		return pFutureContent_->isDone();
	}

	void cancelRequest()
	{
		DLOG(log_ << "cancelRequest" << endl);
		if(pFutureContent_)
		{
			pFutureContent_->cancel(progressConnection_);
			pFutureContent_.reset();
		}
		pFutureContent_=NULL;
	}

	~Future()
	{
		DLOG(log_ << "destructor" << endl);
	}
};

#endif