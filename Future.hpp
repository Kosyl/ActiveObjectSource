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
/**
* It has a pointer to FutureContent which keeps result, progress and state of client request. 
* @brief Future allows a client to obtain the result, progress and state of method invocation.
* @tparam T Type of return value of method invoked.
*/

template<typename T>
class Future
{

private:
	/**
	* Pointer to FutureContent of client request.
	*/
	boost::shared_ptr<FutureContent> pFutureContent_;
	/**
	* 
	*/
	boost::function<void(double)> progressSlot_;
	/**
	* 
	*/
	boost::signals::connection progressConnection_;

	mutable Logger log_;

	void dummyCallback(double x)
	{}

public:
	/**
	* Constructs Future with FutureContent.
	* @param target Pointer to FutureContent that keeps info about client request.
	*/
	Future(boost::shared_ptr<FutureContent> target):
		pFutureContent_(target),
		log_("Future",7),
		progressSlot_(boost::bind(&Future<T>::dummyCallback,this,_1))
	{	
		DLOG(log_ << "constructor" << endl);
		progressConnection_=pFutureContent_->attachProgressObserver(progressSlot_);
	}
	/**
	* Construct copy of other Future.
	* @brief Copy constructor.
	*/ 
	Future(const Future& rhs):
		pFutureContent_(rhs.pFutureContent_),
		log_("Future",7)
	{
		DLOG(log_ << "c constructor" << endl);

		setFunction(rhs.progressSlot_);
		progressConnection_=pFutureContent_->attachProgressObserver(progressSlot_);
		pFutureContent_->cancel(rhs.progressConnection_); //odczepienie sie od starego sygnalu
	}
	/**
	* @brief Destructor
	*/
	~Future()
	{
		DLOG(log_ << "destructor" << endl);
	}
	
	/**
	* @brief Adds slot which is called after progress or state changes.
	* @param fun function that is supposed to be the slot.
	* @tparam FuncType 
	*/
	//- dlaczego template?
	template<typename FuncType>
	void setFunction(FuncType fun)
	{
		DLOG(log_ << "setFunction" << endl);
		progressSlot_=fun;
		boost::signals::connection tmp = pFutureContent_->attachProgressObserver(progressSlot_);
		pFutureContent_->cancel(progressConnection_);
		progressConnection_=tmp;
	}
	/**
	* The method waits until the request is done and then return its value.
	* @brief Returns result of client request.
	* @return result of the client invocation.
	* @throw RequestCancelledException if pFutureContent_ doesn not exist.
	*/
	T getValue() const
	{
		if(!pFutureContent_)
			throw RequestCancelledException();
		DLOG(log_ << "getValue ()" << endl);
		return boost::any_cast<T>(pFutureContent_->getValue());
	}
	/**
	* @brief Converts value of method request into the T value.
	*/
	operator T()
	{
		return getValue();
	}
	/**
	* @brief Returns progress of client request. 
	* @return progress of client request.
	* @throw RequestCancelledException if pFutureContent_ does not exist.
	*/
	double getProgress() const
	{
		if(!pFutureContent_)
			throw RequestCancelledException();
		DLOG(log_ << "getProgress (" << pFutureContent_->getProgress() << ")" << endl);
		return pFutureContent_->getProgress();
	}
	/**
	* @brief Returns exception of FutureContent.
	* @return exception of FutureContent
	* @throw RequestCancelledException if pFutureContent_ does not exist.
	*/
	//--??
	exception getException() const
	{
		if(!pFutureContent_)
			throw RequestCancelledException();
		DLOG(log_ << "getException (" << (pFutureContent_->getException().what()) << ")" << endl);
		return pFutureContent_->getException();
	}
	/**
	* @brief Test whether there is an exception in the client request.
	* @return whether an exception occured in the client request.
	* @throw RequestCancelledException if pFutureContent_ does not exist.
	*/
	bool hasException() const
	{
		if(!pFutureContent_)
			throw RequestCancelledException();
		DLOG(log_ << "hasException (" << pFutureContent_->hasException() << ")" << endl);
		return pFutureContent_->hasException();
	}
	/**
	* @brief Says whether the client request is done.
	* @return whether the request is done.
	* @throw RequestCancelledException if pFutureContent_ does not exist.
	*/
	bool isDone() const
	{
		if(!pFutureContent_)
			throw RequestCancelledException();
		DLOG(log_ << "isDone (" << pFutureContent_->isDone() << ")" << endl);
		return pFutureContent_->isDone();
	}
	/**
	* Disconnets slot function and unpins from the FutureContent.
	* @brief cancel client request.
	*/
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
	/**
	* Assigns other to this Future and returns a reference to this Future.
	* @brief Assignment operator.
	*/ 
	Future<T>& operator=(const Future& rhs)
	{
		DLOG(log_ << "= operator" << endl);
		if(&rhs==this) return *this;
		Future<T> res(*this);
		return res;
	}
};

#endif