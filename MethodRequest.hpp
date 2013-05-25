#ifndef _COMMAND_
#define _COMMAND_

#include <boost\function.hpp>
#include "Future.hpp"

using namespace std;

class NullCommandException: public exception{};

//znow sparametryzowane servantem konkretnym
template<class Servant>
class Functor
{

protected:

	//mamy any, wiec content moze siedziec w klasie bazowej
	boost::shared_ptr<FutureContent> content_;

public:

	Functor(){}
	virtual ~Functor();

	virtual void execute(Servant*)=0;

	//zwraca wskaznik do contentu, zeby powiedziec servantowi w co ma celowac
	//MethodRequest uzywa tego wskaznika do ustawiania wyjatkow i setValue
	//ale progress siedzi w funkcji, wiec ten sam wskaznik jest przekazywany do Servanta
	//zeby mogl na nim (na tym samym contencie) wywolywac setProgress()
	boost::shared_ptr<FutureContent> getFutureContent()
	{
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
		command_(f),
		content_(content)
	{}

	//Scheduler przekaze tu wskaznik na servanta
	//przy czym bedzie to juz wskaznik na konkretna klase, a nie bazowa, dzieki parametrowi w szablonie
	//wiec command bedzie szukalo funkcji we wlasciwej klasie
	//Servant ma swoj wskaznik na ten sam content, i wewnatrz funkcji moze ustawiac progress
	virtual void execute(Servant* servant)
	{
		if(isReady())
		{
			try
			{
				content_->setValue(command_(servant));
			}
			catch(RequestCancelledException& rc)
			{
				content_->setCancelled();
			}
			catch(...)
			{
				content_->setException(boost::current_exception());
			}
		}
		else
		{
			throw NullCommandException();
		}
	}

	virtual bool isReady()
	{
		return (command_!=false && content_!=NULL);
	}
};

#endif