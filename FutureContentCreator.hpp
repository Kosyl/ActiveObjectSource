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
		log_("FC CREATOR")
	{
		log_ << "constructor" << endl;
	}

	FutureContentCreator(const FutureContentCreator& rhs):
		log_("FC CREATOR"),
		pFutureContent_(rhs.pFutureContent_)
	{
		log_ << "c constructor" << endl;
	}

	virtual ~FutureContentCreator()
	{
		log_ << "~FutureContentCreator()" << endl;
	}

	void setFutureContent(boost::shared_ptr<FutureContent> futureContentPtr)
	{
		log_ << "setFutureContent ()" << endl;
		pFutureContent_=futureContentPtr;
	}

protected:

	void setProgress(const double& progress)
	{
		log_ << "setProgress (" << progress << ")" << endl;
		pFutureContent_->setProgress(progress);
	}

	void setState(const FutureState fs)
	{
		log_ << "setState (" << fs << ")" << endl;
		pFutureContent_->setState(fs);
	}

	void setException(boost::exception_ptr& e)
	{
		log_ << "setException ()" << endl;
		pFutureContent_->setException(e);
	}

	template<typename T>
	void setValue(const T& val)
	{
		log_ << "setValue (" << val << ")" << endl;
		pFutureContent_->setValue(val);
	}

	bool isCancelled()
	{
		log_ << "isCancelled (" << pFutureContent_->isCancelled() << ")" << endl;
		return pFutureContent_->isCancelled();
	}

	void setCancelled()
	{
		log_ << "setException ()" << endl;
		pFutureContent_->setException(boost::copy_exception(new RequestCancelledException()));
		pFutureContent_->setState(FutureState::CANCELLED);
	}

	Logger log_;

private:

	boost::shared_ptr<FutureContent> pFutureContent_;
};

#endif
