#ifndef _FUTURE_CONTENT_
#define _FUTURE_CONTENT_

#include <boost/shared_ptr.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/ref.hpp>

/**
* @enum 
* @brief State of the method invocation.
*/
enum FutureState 
{
	QUEUED, /**<Method request waits in the queue*/
	CANCELLED, /**<Method request is cancelled*/
	INPROGRESS, /**<Method request is now executed*/
	EXCEPTION, /**<An exception occured by the method execution*/
	DONE /**<Method request is done*/
};
/**
* @brief RequestCancelledException ???????????????????dopisze jak ulozy mi sie w glowie ladne zdanie
*/
class RequestCancelledException: public exception
{
public:
/**
* @brief Constructor.
*/
	RequestCancelledException():
	exception("Request was cancelled by user")
	{}
};
/**
* It is read by Future and written by FutureContentCreator.
* @brief FutureContent keeps result, progress and state of method invocation.
*/
class FutureContent: public boost::noncopyable
{
private:
	/**
	* Result value of method invoction.
	*/
	boost::any value_;
	/**
	* Exception from the method invocation.
	*/
	boost::exception_ptr exception_;
	/**
	* Progress of method execution.
	*/
	double progress_;
	/**
	* State of method invocation.
	*/
	volatile FutureState state_;

	mutable boost::mutex mutex_;
	boost::condition_variable waitingFutures_;
	/**
	* Signal that notifies observers when the progress is changed.
	*/
	boost::signal<void(double)> notifyObservers_;
	/**
	* Logger
	*/
	mutable Logger log_;

public:

	typedef boost::mutex::scoped_lock sLock;
	/**
	* It sets progress to 0, state to FutureState::QUEUED and exception to NULL.
	* @brief Constructor.
	*/
	FutureContent():
		state_(FutureState::QUEUED),
		progress_(0.0),
		exception_(NULL),
		log_("Content",6)
	{
		DLOG(log_ << "constructor" << endl);
	}
	/**
	* After that notifies observers.
	* @brief Sets new progress value.
	* @param progress New progress value.
	*/
	void setProgress(const double& progress)
	{
		sLock lock(mutex_);
		DLOG(log_ << "setProgress (" << progress << ")" << endl);
		progress_ = progress;
		notifyObservers_(progress);
	}
	/**
	* @brief Returns progress.
	* @return progress
	*/
	double getProgress() const
	{
		sLock lock(mutex_);
		DLOG(log_ << "getProgress (" << progress_ << ")" << endl);
		return progress_;
	}
	/**
	* @brief Test whether the method invocation is done.
	* @return whether the method execution is finished.
	*/
	bool isDone() const
	{
		sLock lock(mutex_);
		DLOG(log_ << "isDone (" << (state_==FutureState::DONE?true:false) << ")" << endl);
		return (state_==FutureState::DONE?true:false);
	}
	/**
	* @brief Test whether there is an exception in the client request.
	* @return whether an exception occured in the client request.
	*/
	bool hasException() const
	{
		sLock lock(mutex_);
		DLOG(log_ << "hasException (" << (exception_!=NULL) << ")" << endl);
		return exception_!=NULL;//? ciekawe czy wystarczy, trzeba jakos zrobic rzucanie
	}
	/**
	* @brief Returns exception of the method invocation.
	* @return exception of the method invocation.
	*/
	boost::exception_ptr getException()
	{
		sLock lock(mutex_);
		DLOG(log_ << "getException ()" << endl);
		return exception_;//? ciekawe czy wystarczy, trzeba jakos zrobic rzucanie
	}
	/**
	* @brief Sets exception of the method invocation.
	* @param e Pointer to the exception.
	*/
	void setException(boost::exception_ptr& e)
	{
		sLock lock(mutex_);
		DLOG(log_ << "set exception: ()" << endl);
		state_=FutureState::EXCEPTION;
		exception_=e;
	}
	/**
	* If there is no more observers of the request, it sets state to FutureState::CANCELLED.
	* @brief Cancel connection with the given connection.
	* @param c Connection to be cancelled.
	*/
	void cancel(boost::signals::connection c)
	{
		sLock lock(mutex_);
		
		if(c.connected())
		{
			c.disconnect();
		}
		DLOG(log_ << "cancel, remaining observers: " << notifyObservers_.num_slots() << endl);
		if(notifyObservers_.empty())
		{
			state_ = FutureState::CANCELLED;
			DLOG(log_ << "cancel - brak obserwatorow, ustawiam cancel" << endl);
		}
		//exception_ = boost::copy_exception(new RequestCancelledException());
	}
	/**
	* @brief Indicates whether the request is cancelled.
	* @return whether the request is cancelled.
	*/
	bool isCancelled() const
	{
		sLock lock(mutex_);
		DLOG(log_ << "isCancelled (" << (state_==FutureState::CANCELLED?true:false) << ")" << endl);
		return (state_==FutureState::CANCELLED?true:false);
	}

	int getNumObservers() const
	{
		return notifyObservers_.num_slots();
	}

	/**
	* @brief Attaches progress observer.
	* @param call slot that is called when the progress changes.
	* @return boost::signals::connection with given slot.
	*/
	boost::signals::connection attachProgressObserver(boost::function<void(double)>& call)
	{
		DLOG(log_ << "attachProgressObserver" << endl);
		return notifyObservers_.connect(call);
	}

	/**
	* @brief Sets state of the method invocation.
	* @param fs new state of method invocation.
	*/
	void setState(const FutureState& fs)
	{
		DLOG(log_ << "setState" << endl);
		state_=fs;
	}
	/**
	* After setting a value it sets progress to 1.0, state to FutureState::DONE and notifies all the observers.
	* @brief Sets value of method request.
	* @param val result of method invocation.
	*/
	template<typename T>
	void setValue(const T& val) 
	{
		sLock lock(mutex_);
		DLOG(log_ << "setValue (" << val << ")" << endl);
		value_=val;
		state_=FutureState::DONE;
		progress_=1.0;
		waitingFutures_.notify_all();
	}

	/**
	* If the request is queued or in progress, it waits until method is done. When the state is FutureState::EXCEPTION, it rethrows exception. s
	* @brief Return value of the method invocation.
	* @return value of the method invocation.
	*/
	//- a kiedy CANCELLED?
	virtual boost::any getValue()//powinno blokowac, az bedzie ok; na razie nie mamy wielu w¹tków, wiêc zobaczymy jak to dzia³a za jakieœ 2 miesi¹ce kiedy je w koñcu dodamy
	{
		sLock lock(mutex_);
		DLOG(log_ << "getValue" << endl);
		if(exception_)
		{
			DLOG(log_ << "getValue - exception!" << endl);
			boost::rethrow_exception(exception_);
		}

		while(!(state_==FutureState::DONE || state_==FutureState::CANCELLED || state_==FutureState::EXCEPTION))
		{
			DLOG(log_ << "getValue czeka..." << endl);
			waitingFutures_.wait(lock);
		}

		if(exception_)
		{
			DLOG(log_ << "getValue - exception!" << endl);
			boost::rethrow_exception(exception_);
		}

		boost::any res = value_;
		return res;
	}
	/**
	* @brief Destructor
	*/
	~FutureContent()
	{
		DLOG(log_ << "destructor" << endl);
	}


};

#endif