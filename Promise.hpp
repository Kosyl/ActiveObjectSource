#ifndef _PROMISE_
#define _PROMISE_

#include "Future.hpp"
#include <boost\shared_ptr.hpp>
#include "SimpleLog.hpp"

class Promise
{
private:
	Logger log_;
	boost::shared_ptr<FutureContent> pFutureContent_;

public:
	Promise():
		log_("PROMISE"),
		pFutureContent_(new FutureContent())
	{
		log_ << "constructor" << endl;
	}

	Future getFuture()
	{
		log_ << "getFuture" << endl;
		Future res(pFutureContent_);
		return res;
	}

	void setProgress(const double& progress)
	{
<<<<<<< HEAD
		log_ << "setProgress (" << progress << ")" << endl;
		pFutureContent_->setProgress(progress);
=======
		log << "Set progress: " << progress << endl;
		future_->setProgress(progress);  //literowka :P
>>>>>>> 054c11be264a0b2327a58fdf80e4896358b51bda
	}

	template<typename T>
	void setValue(const T& val)
	{
		log_ << "setValue (" << val << ")" << endl;
		pFutureContent_->setValue(val);
	}

	bool isCancelled()
	{
		log_ << "iscancelled (" << pFutureContent_->isCancelled() << ")" << endl;
		return pFutureContent_->isCancelled();
	}

	~Promise()
	{
		log_ << "~Promise()" << endl;
	}

};

#endif