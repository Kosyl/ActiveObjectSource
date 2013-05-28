#ifndef _COMMAND_
#define _COMMAND_

#include <boost\function.hpp>
#include "Future.hpp"
#include "SimpleLog.hpp"

using namespace std;

class NullCommandException: public exception{};

//znow sparametryzowane servantem konkretnym
template<class Servant>
class Functor
{

protected:
	Logger log_;
	//mamy any, wiec content moze siedziec w klasie bazowej
	boost::shared_ptr<FutureContent> content_;

public:

	Functor(boost::shared_ptr<FutureContent> content):
		content_(content),
		log_("Functor",8)
	{}

	virtual ~Functor()
	{
	}

	virtual void execute(Servant* servant)=0;

	//zwraca wskaznik do contentu, zeby powiedziec servantowi w co ma celowac
	//MethodRequest uzywa tego wskaznika do ustawiania wyjatkow i setValue
	//ale progress siedzi w funkcji, wiec ten sam wskaznik jest przekazywany do Servanta
	//zeby mogl na nim (na tym samym contencie) wywolywac setProgress()
	boost::shared_ptr<FutureContent> getFutureContent()
	{
		DLOG(log_ << "getFutureContent()" << endl);
		return content_;
	}

	//moze sie przyda, moze nie
	virtual bool isReady()=0;
};

//2 parametry! reszta siedzi w b::function i b::bind
template<typename ReturnType, class Servant>
class MethodRequest:public Functor<Servant>
{

private:

	boost::function<ReturnType(Servant*)> command_;

public:

	MethodRequest(boost::function<ReturnType(Servant*)> f, boost::shared_ptr<FutureContent> content):
		Functor<Servant>(content),
		command_(f)
	{
		DLOG(log_ << "constructor" << endl);
	}

	//Scheduler przekaze tu wskaznik na servanta
	//przy czym bedzie to juz wskaznik na konkretna klase, a nie bazowa, dzieki parametrowi w szablonie
	//wiec command bedzie szukalo funkcji we wlasciwej klasie
	//Servant ma swoj wskaznik na ten sam content, i wewnatrz funkcji moze ustawiac progress
	virtual void execute(Servant* servant)
	{
		DLOG(log_ << "execute() - begin" << endl);
		if(isReady())
		{
			try
			{
				DLOG(log_ << "execute() - isReady==true, executing..." << endl);
				content_->setValue(command_(servant));
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

	virtual bool isReady()
	{
		return (command_!=false && content_!=NULL);
	}

	virtual ~MethodRequest()
	{
		DLOG(log_ << "destructor" << endl);
	}

};

#endif