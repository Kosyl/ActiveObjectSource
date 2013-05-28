#ifndef _FC_CREATOR_
#define _FC_CREATOR_

#include "Future.hpp"
#include <boost\shared_ptr.hpp>
#include <boost\exception_ptr.hpp>
#include "SimpleLog.hpp"
#include "FutureContent.hpp"

//to jest taki poprzedni Promise
//teraz dziedziczy po nim kazdy Servant, wiec zmienilem nazwe na bardziej wymowna
//czyli to taki "pisarz" contentu
class FutureContentCreator
{

public:

	FutureContentCreator():
		log_("Creator",5)
	{
		DLOG(log_ << "constructor" << endl);
	}

	FutureContentCreator(const FutureContentCreator& rhs):
		log_("Creator"),
		pFutureContent_(rhs.pFutureContent_)
	{
		DLOG(log_ << "c constructor" << endl);
	}

	virtual ~FutureContentCreator()
	{
		DLOG(log_ << "destructor" << endl);
	}

	void setFutureContent(boost::shared_ptr<FutureContent> futureContentPtr)
	{
		DLOG(log_ << "setFutureContent ()" << endl);
		pFutureContent_=futureContentPtr;
	}

protected:

	void setProgress(const double& progress)
	{
		DLOG(log_ << "setProgress (" << progress << ")" << endl);
		pFutureContent_->setProgress(progress);
	}

	void setState(const FutureState fs)
	{
		DLOG(log_ << "setState (" << fs << ")" << endl);
		pFutureContent_->setState(fs);
	}

	void setException(boost::exception_ptr& e)
	{
		DLOG(log_ << "setException ()" << endl);
		pFutureContent_->setException(e);
	}

	template<typename T>
	void setValue(const T& val)
	{
		DLOG(log_ << "setValue (" << val << ")" << endl);
		pFutureContent_->setValue(val);
	}

	bool isCancelled()
	{
		DLOG(log_ << "isCancelled ()" << endl);
		return pFutureContent_->isCancelled();
	}

	void setCancelled()
	{
		DLOG(log_ << "setException ()" << endl);
		//pFutureContent_->setException(boost::copy_exception(new RequestCancelledException()));
		pFutureContent_->setState(FutureState::CANCELLED);
	}

	Logger log_;

private:

	boost::shared_ptr<FutureContent> pFutureContent_;
};

#endif
