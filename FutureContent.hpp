#ifndef _FUTURE_CONTENT_
#define _FUTURE_CONTENT_

#include <boost/shared_ptr.hpp>
#include <boost/ref.hpp>

enum FutureState {QUEUED, CANCELLED, INPROGRESS, EXCEPTION, DONE};

class RequestCancelledException: public exception
{
public:
	RequestCancelledException(){}
};

class FutureContent: public boost::noncopyable
{

private:
	boost::any value_;
	exception* exception_;
	double progress_;
	volatile FutureState state_;

	mutable boost::mutex mutex_;
	boost::condition_variable waitingFutures_;

	boost::signal<void(double)> notifyObservers_;
	mutable Logger log_;

public:
	typedef boost::mutex::scoped_lock sLock;

	FutureContent():
		state_(FutureState::QUEUED),
		progress_(0.0),
		exception_(NULL),
		log_("F. CONTENT")
	{
		log_ << "constructor" << endl;
	}

	void setProgress(const double& progress)
	{
		sLock lock(mutex_);
		log_ << "setProgress (" << progress << ")" << endl;
		progress_ = progress;
		notifyObservers_(progress);
	}

	double getProgress() const
	{
		sLock lock(mutex_);
		log_ << "getProgress (" << progress_ << ")" << endl;
		return progress_;
	}

	bool isDone() const
	{
		sLock lock(mutex_);
		log_ << "isDone (" << (state_==FutureState::DONE?true:false) << ")" << endl;
		return (state_==FutureState::DONE?true:false);
	}

	bool hasException() const
	{
		sLock lock(mutex_);
		log_ << "hasException (" << (exception_!=NULL) << ")" << endl;
		return exception_!=NULL;//? ciekawe czy wystarczy, trzeba jakos zrobic rzucanie
	}

	exception getException()
	{
		sLock lock(mutex_);
		log_ << "getException (" << (exception_->what()) << ")" << endl;
		return *exception_;//? ciekawe czy wystarczy, trzeba jakos zrobic rzucanie
	}

	void cancel(boost::signals::connection c)
	{
		sLock lock(mutex_);
		log_ << "cancel" << endl;
		if(c.connected())
		{
			c.disconnect();
		}

		if(notifyObservers_.empty())
		{
			state_ = FutureState::CANCELLED;
			log_ << "cancel - brak obserwatorow, ustawiam cancel" << endl;
		}
		exception_ = new RequestCancelledException();
	}

	void setException(const exception& e)
	{
		sLock lock(mutex_);
		cout << "set exception: " << e.what() << endl;
		state_ = FutureState::EXCEPTION;
		exception_ = new exception(e);
	}

	bool isCancelled() const
	{
		sLock lock(mutex_);
		log_ << "isCancelled (" << (state_==FutureState::CANCELLED?true:false) << ")" << endl;
		return (state_==FutureState::CANCELLED?true:false);
	}

	boost::signals::connection attachProgressObserver(boost::function<void(double)>& call)
	{
		log_ << "attachProgressObserver" << endl;
		return notifyObservers_.connect(call);//sprawdzic czy ok
	}

	void dettachProgressObserver(const boost::signals::connection& progressConnection_)
	{
		log_ << "dettachProgressObserver" << endl;
		progressConnection_.disconnect();//sprawdzic czy ok
	}


	template<typename T>
	void setValue(const T& val) 
	{
		sLock lock(mutex_);
		log_ << "setValue (" << val << ")" << endl;
		value_=val;
		state_=FutureState::DONE;
		progress_=1.0;
		exception_=NULL;
		waitingFutures_.notify_all();
	}

	virtual boost::any getValue()//powinno blokowac, az bedzie ok; na razie nie mamy wielu w�tk�w, wi�c zobaczymy jak to dzia�a za jakie� 2 miesi�ce kiedy je w ko�cu dodamy
	{
		sLock lock(mutex_);
		log_ << "getValue" << endl;
		if(exception_)
		{
			throw exception_;
		}

		while(!(state_==FutureState::DONE || state_==FutureState::CANCELLED || state_==FutureState::EXCEPTION))
		{
			log_ << "getValue czeka..." << endl;
			waitingFutures_.wait(lock);
		}

		if(exception_)
		{
			throw exception_;
		}

		boost::any res = value_;
		return res;
	}

	~FutureContent()
	{
		log_ << "~FutureContent()" << endl;
	}

};

#endif