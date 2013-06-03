/**
* @file Future.hpp
* @author Michal Kosyl
* @author Marta Kuzak
* @details Active Object implementation.
* @details Future allows a client to obtain the state, progress and results of method invocations.
*/
#ifndef _FUTURE_
#define _FUTURE_

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/thread.hpp>
#include "SimpleLog.hpp"
#include <boost/noncopyable.hpp>
#include <boost/signal.hpp>
#include <boost/any.hpp>
#include <vector>
#include "FutureContent.hpp"

using namespace std;

namespace ActiveObject
{
    /**
     * Basis class for Futures
     * It has a pointer to FutureContent which keeps result, progress and state of client request. 
     * @brief Basis class for Futures
     * @tparam T Type of return value of method invoked.
     */
    class FutureBase
    {
    protected:
	
	/**
	 * Pointer to FutureContent of client request.
	 */
	boost::shared_ptr<FutureContent> pFutureContent_;
	
	/**
	 * Function to be called on progress update
	 */
	boost::function<void(double)> progressSlot_;
	
	/**
	 * Connection to the progress notification signal
	 */
	boost::signals::connection progressConnection_;
	/**
	* Thread-safe logger.
	*/
	mutable Logger log_;
	
	/**
	 * An empty callback, used to identify Futures observing the FutureContent
	 */
	void dummyCallback(double x)
	{}
	
    public:
	
	/**
	 * Constructor. Binds the default (empty) progress notification slot to the FutureContent
	 * @param target pointer to FutureContent
	 */
	FutureBase(boost::shared_ptr<FutureContent> target):
	pFutureContent_(target),
	log_("Future",7)
	{	
	    //DLOG(log_ << "constructor" << endl);
		progressSlot_=boost::bind(&FutureBase::dummyCallback,this,_1);
	    progressConnection_=pFutureContent_->attachProgressObserver(progressSlot_);
	}
	
	/**
	 * Copy constructor. Binds the default (empty) progress notification slot to the FutureContent.
	 * Copies the progress slot function as well.
	 */
	FutureBase(const FutureBase& rhs):
	pFutureContent_(rhs.pFutureContent_),
	log_("Future",7)
	{
	    //DLOG(log_ << "c constructor" << endl);
	    
	    progressSlot_=rhs.progressSlot_;
	    progressConnection_=pFutureContent_->attachProgressObserver(progressSlot_);
	}
	
	/**
	 * Destructor
	 * Notifies the FutureContent to decrement the number of Observers
	 */
	virtual ~FutureBase()
	{
	    DLOG(log_ << "destructor" << endl);
	    if(pFutureContent_)
	    {
		if(pFutureContent_->getNumObservers()==1)
		{
		    DLOG(log_ << "tylko rozlaczam" << endl);
		    progressConnection_.disconnect();
		}
		else
		{
		    DLOG(log_ << "robie cancel na contencie" << endl);
		    pFutureContent_->cancel(progressConnection_);
		}
		pFutureContent_.reset();
	    }
	}
	
	/**
	 * @brief Adds slot which is called after progress or state changes.
	 * @param fun function (or any object providing operator()(double) ) that is supposed to be the slot.
	 * @tparam FuncType 
	 */
	template<typename FuncType>
	void setFunction(FuncType fun)
	{
	    DLOG(log_ << "setFunction" << endl);
	    
	    progressSlot_=fun;
	    if(!progressSlot_)progressSlot_=boost::bind(&FutureBase::dummyCallback,this,_1);
	    boost::signals::connection tmp = pFutureContent_->attachProgressObserver(progressSlot_);
	    pFutureContent_->cancel(progressConnection_);
	    progressConnection_=tmp;
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
	    //DLOG(log_ << "getProgress (" << pFutureContent_->getProgress() << ")" << endl);
		return pFutureContent_->getProgress();
	}
	
	/**
	 * @brief Returns pointer to exception that happened during request execution, if any.
	 * @return exception of FutureContent
	 * @throw RequestCancelledException if pFutureContent_ does not exist.
	 */
	boost::exception_ptr getException() const
	{
	    if(!pFutureContent_)
		throw RequestCancelledException();
	    //DLOG(log_ << "getException ()" << endl);
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
	    //DLOG(log_ << "hasException (" << pFutureContent_->hasException() << ")" << endl);
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
	    //DLOG(log_ << "isDone (" << pFutureContent_->isDone() << ")" << endl);
		return pFutureContent_->isDone();
	}
	
	/**
	 * Disconnets slot function and unpins from the FutureContent.
	 * @brief cancel client request.
	 */
	void cancelRequest()
	{
	    //DLOG(log_ << "cancelRequest" << endl);
	    if(pFutureContent_)
	    {
		pFutureContent_->cancel(progressConnection_);
		pFutureContent_.reset();
	    }
	    pFutureContent_=NULL;
	}
    };
    
    /**
     * It has a pointer to FutureContent which keeps result (derived from the base), progress and state of client request.
     * Also, the template specifies the type of return value.
     * @brief Future allows a client to obtain the result, progress and state of method invocation.
     * @tparam T Type of return value of method invoked.
     */
    template<typename T>
    class Future: public FutureBase
    {
	
    public:
	
	/**
	 * Constructs Future pointing to the given FutureContent.
	 * @param target Pointer to FutureContent that keeps info about client request.
	 */
	Future(boost::shared_ptr<FutureContent> target):
	FutureBase(target)
	{	
	}
	
	/**
	 * Construct copy of other Future (pointing to the same FutureContent and with the same progress slot).
	 * @brief Copy constructor.
	 */ 
	Future(const Future<T>& rhs):
	FutureBase(rhs)
	{
	}
	
	/**
	 * @brief Destructor
	 */
	~Future()
	{
	}
	
	/**
	 * The method waits until the request is done and then return its value.
	 * @brief Returns result of client request.
	 * @return result of the client invocation.
	 * @throw RequestCancelledException if pFutureContent_ does not exist.
	 */
	T getValue() const
	{
	    if(!pFutureContent_)
			throw RequestCancelledException();
	    //DLOG(log_ << "getValue ()" << endl);
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
	 * Assigns other to this Future and returns a reference to this Future.
	 * @brief Assignment operator.
	 */ 
	Future<T>& operator=(const Future& rhs)
	{
	    //DLOG(log_ << "c constructor" << endl);
	    progressSlot_=rhs.progressSlot_;
	    boost::signals::connection tmp=rhs.pFutureContent_->attachProgressObserver(progressSlot_);
	    pFutureContent_->cancel(progressConnection_);
	    progressConnection_=tmp;
	    pFutureContent_=rhs.pFutureContent_;
	    return *this;
	}
    };
    
    /**
     * Void-type specialization of Future.
     * 
     * @brief Future allows a client to obtain the result, progress and state of method invocation.
     */
    template<>
    class Future<void>: public FutureBase
    {
	
    public:
	/**
	 * Constructs Future with FutureContent.
	 * @param target Pointer to FutureContent that keeps info about client request.
	 */
	Future(boost::shared_ptr<FutureContent> target):
	FutureBase(target)
	{	
	}
	
	/**
	 * Construct copy of other Future.
	 * @brief Copy constructor.
	 */ 
	Future(const Future& rhs):
	FutureBase(rhs)
	{
	}
	
	/**
	 * @brief Destructor. Because the user may not want to keep the Future<void> object,
	 * the destructor detects if this Future was destroyed, but not cancelled, and if so, prevents the execution form stopping.
	 */
	~Future()
	{
	}
	
	/**
	 * The method waits until the request is done and then return its value.
	 * @brief Returns result of client request.
	 * @return True, if the void request was completed successfully.
	 * @throw RequestCancelledException if pFutureContent_ doesn not exist (e.g. was cancelled by te user).
	 */
	bool getValue() const
	{
	    if(!pFutureContent_)
			throw RequestCancelledException();
	    //DLOG(log_ << "getValue ()" << endl);
		return boost::any_cast<bool>(pFutureContent_->getValue());
	}
	
	/**
	 * @brief Converts value of method request into true, if the method id completed
	 */
	operator bool()
	{
	    return getValue();
	}
	
	/**
	 * Assigns other to this Future and returns a reference to this Future.
	 * @brief Assignment operator.
	 */ 
	Future& operator=(const Future& rhs)
	{
	    //DLOG(log_ << "c constructor" << endl);
	    progressSlot_=rhs.progressSlot_;
	    boost::signals::connection tmp=rhs.pFutureContent_->attachProgressObserver(progressSlot_);
	    pFutureContent_->cancel(progressConnection_);
	    progressConnection_=tmp;
	    pFutureContent_=rhs.pFutureContent_;
	    return *this;
	}
    };
    
}//ActiveObject
#endif
