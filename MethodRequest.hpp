#ifndef _COMMAND_
#define _COMMAND_

#include <boost\function.hpp>
#include "Future.hpp"
#include "SimpleLog.hpp"

using namespace std;
/**
* @brief NullCommandException
*/
class NullCommandException: public exception{};

/**
* @brief An abstract class which defines an interface for executing methods.
* @tparam Servant Type of Servant that executes method.
*/
//znow sparametryzowane servantem konkretnym
template<class Servant>
class Functor
{

protected:
	Logger log_;
	//mamy any, wiec content moze siedziec w klasie bazowej
	/**
	* Pointer to FutureContent which contains result, progress and state of method invocation.
	*/
	//-œmieszne to w sumie jest invoke, invocation
	boost::shared_ptr<FutureContent> content_;
	auto_ptr<boost::function<bool(Servant*)> > guard_;

public:
	/**
	* @brief Constructs Functor with a given FutureContent
	* @param content Pointer to FutureContent
	*/
	Functor(boost::shared_ptr<FutureContent> content):
		content_(content),
		log_("Functor",8),
		guard_(NULL)
	{}

	Functor(boost::shared_ptr<FutureContent> content, boost::function<bool(Servant*)> guard):
		content_(content),
		log_("Functor",8),
		guard_(new boost::function<bool(Servant*)>)
	{
		*guard_=guard;
	}
	/**
	* Destructor
	*/
	virtual ~Functor()
	{
	}
	/**
	* @brief Makes given servant execute method.
	* @param Pointer to servant
	*/
	virtual void execute(boost::shared_ptr<Servant> servant)=0;

	//zwraca wskaznik do contentu, zeby powiedziec servantowi w co ma celowac
	//MethodRequest uzywa tego wskaznika do ustawiania wyjatkow i setValue
	//ale progress siedzi w funkcji, wiec ten sam wskaznik jest przekazywany do Servanta
	//zeby mogl na nim (na tym samym contencie) wywolywac setProgress()
	/**
	* @brief Returns pointer to FutureContent.
	* @return pointer to FutureContent that contains info about the method invocation.
	*/
	boost::shared_ptr<FutureContent> getFutureContent()
	{
		DLOG(log_ << "getFutureContent()" << endl);
		return content_;
	}

	bool guard(boost::shared_ptr<Servant> s)
	{
		DLOG(log_ << "guard" << endl);
		if(guard_.get()==NULL) return false;
		
		return (*guard_)(s.get());
	}

	//moze sie przyda, moze nie
	/**
	*
	*/
	virtual bool isReady()=0;
};

//2 parametry! reszta siedzi w b::function i b::bind
/**
* @brief Implements Functor interface.
* @tparam ReturnType Type of return value of the invoked method.
* @tparam Servant Type of servant that executes method.
*/
template<typename ReturnType, class Servant>
class MethodRequest:public Functor<Servant>
{

private:
	/**
	* Pointer to command
	*/
	boost::function<ReturnType(Servant*)> command_;

public:
	/**
	* @brief Constructs MethodRequest with given command and FutureContent.
	* @param f Invoked command
	* @param content Pointer to FutureContent that contains info about the invoked command.
	*/
	MethodRequest(boost::function<ReturnType(Servant*)> f, boost::shared_ptr<FutureContent> content):
		Functor<Servant>(content),
		command_(f)
	{
		DLOG(log_ << "constructor" << endl);
	}

	MethodRequest(boost::function<ReturnType(Servant*)> f, boost::shared_ptr<FutureContent> content, boost::function<bool(Servant*)> guard):
		Functor<Servant>(content, guard),
		command_(f)
	{
		DLOG(log_ << "constructor" << endl);
	}

	//Scheduler przekaze tu wskaznik na servanta
	//przy czym bedzie to juz wskaznik na konkretna klase, a nie bazowa, dzieki parametrowi w szablonie
	//wiec command bedzie szukalo funkcji we wlasciwej klasie
	//Servant ma swoj wskaznik na ten sam content, i wewnatrz funkcji moze ustawiac progress
	/**
	* @brief Implementation of Functor::execute
	* @param servant Servant that is to execute the method.
	* @see virtual void Functor::execute(boost::shared_ptr<Servant> servant)
	* @throw NullCommandException when the method is not ready.
	*/
	virtual void execute(boost::shared_ptr<Servant> servant)
	{
		DLOG(log_ << "execute() - begin" << endl);
		if(isReady())
		{
			try
			{
				DLOG(log_ << "execute() - isReady==true, executing..." << endl);
				content_->setValue(command_(servant.get()));
			}
			catch(RequestCancelledException)
			{
				DLOG(log_ << "execute() - request cancelled" << endl);
			}
			catch(...)
			{
				DLOG(log_ << "execute() - exception" << endl);
				content_->setException(boost::current_exception());
			}
			DLOG(log_ << "execute() - finished" << endl);
		}
		else
		{
			DLOG(log_ << "execute() - isReady==false" << endl);
			throw NullCommandException();
		}
	}
	/**
	* @brief Says if the MethodRequest is ready.
	* @return whether MethodRequest is ready.
	*/
	virtual bool isReady()
	{
		return (command_!=false && content_!=NULL);
	}
	/**
	* Destructor.
	*/
	virtual ~MethodRequest()
	{
		DLOG(log_ << "destructor" << endl);
	}

};

template<class Servant>
class MethodRequest<void,Servant>:public Functor<Servant>
{

private:
	/**
	* Pointer to command
	*/
	boost::function<void(Servant*)> command_;

public:
	/**
	* @brief Constructs MethodRequest with given command and FutureContent.
	* @param f Invoked command
	* @param content Pointer to FutureContent that contains info about the invoked command.
	*/
	MethodRequest(boost::function<void(Servant*)> f, boost::shared_ptr<FutureContent> content):
		Functor<Servant>(content),
		command_(f)
	{
		DLOG(log_ << "constructor" << endl);
	}

	MethodRequest(boost::function<void(Servant*)> f, boost::shared_ptr<FutureContent> content, boost::function<bool(Servant*)> guard):
		Functor<Servant>(content, guard),
		command_(f)
	{
		DLOG(log_ << "constructor" << endl);
	}

	//Scheduler przekaze tu wskaznik na servanta
	//przy czym bedzie to juz wskaznik na konkretna klase, a nie bazowa, dzieki parametrowi w szablonie
	//wiec command bedzie szukalo funkcji we wlasciwej klasie
	//Servant ma swoj wskaznik na ten sam content, i wewnatrz funkcji moze ustawiac progress
	/**
	* @brief Implementation of Functor::execute
	* @param servant Servant that is to execute the method.
	* @see virtual void Functor::execute(boost::shared_ptr<Servant> servant)
	* @throw NullCommandException when the method is not ready.
	*/
	virtual void execute(boost::shared_ptr<Servant> servant)
	{
		DLOG(log_ << "execute() - begin" << endl);
		if(isReady())
		{
			try
			{
				DLOG(log_ << "execute() - isReady==true, executing..." << endl);
				command_(servant.get());
				content_->setValue(true);
			}
			catch(RequestCancelledException)
			{
				DLOG(log_ << "execute() - request cancelled" << endl);
			}
			catch(...)
			{
				DLOG(log_ << "execute() - exception" << endl);
				content_->setException(boost::current_exception());
			}
			DLOG(log_ << "execute() - finished" << endl);
		}
		else
		{
			DLOG(log_ << "execute() - isReady==false" << endl);
			throw NullCommandException();
		}
	}
	/**
	* @brief Says if the MethodRequest is ready.
	* @return whether MethodRequest is ready.
	*/
	virtual bool isReady()
	{
		return (command_!=false && content_!=NULL);
	}
	/**
	* Destructor.
	*/
	virtual ~MethodRequest()
	{
		DLOG(log_ << "destructor" << endl);
	}

};

#endif