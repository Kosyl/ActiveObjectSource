#ifndef _EXAMPLE1_
#define _EXAMPLE1_

#include "SimpleLog.hpp"
#include "FutureContentCreator.hpp"
#include <boost\function.hpp>
#include "Proxy.hpp"
#include "Future.hpp"
#include "MethodRequest.hpp"

using namespace std;

//implementacja przykladowego servanta, ma progress!
class CalcServant: public FutureContentCreator
{

public:

	CalcServant()
	{}

	CalcServant(const CalcServant& rhs)
	{}

	int AddInt(int a, int b)
	{
		setProgress(0.2);
		setProgress(0.8);
		return a+b;
	}

	int ReallyFrickinLongAddInt(int a, int b)
	{
		for(double i=0;i<1.0;i+=0.2)
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
			setProgress(i);
			if(isCancelled()) 
				throw RequestCancelledException();
		}

		return a+b;
	}

	int DivideInt(int a, int b) throw(...)
	{
		setProgress(0.2);
		if (b == 0)
			throw std::overflow_error("Divide by zero exception");
		setProgress(0.8);
		return a/b;
	}
};

//proxy do servanta
//mowimy, jaki typ servanta i jak jest generowany dla kazdego schedulera
class CalcProxy: public Proxy<CalcServant,ServantFactoryCreator>
{

public:

	CalcProxy(int numThreads=1):
		Proxy(numThreads)
	{}

	//tyle kodu bedzie potrzeba na obsluge zadania jednej funkcji
	//bardzo mozliwe ze da sie to zautomatyzowac
	Future<int> AddInt(int a, int b)
	{
		//pusty content
		DLOG(log_ << "Add() - content creation" << endl);
		boost::shared_ptr<FutureContent> pContent(new FutureContent());

		//future do contentu
		DLOG(log_ << "Add() - future creation" << endl);
		Future<int> fut(pContent);

		//zbindowanie odpowiedniej metody, pozostawienie argumentu na wskaznik do konkretnego servanta
		DLOG(log_ << "Add() - binding" << endl);
		boost::function<int(CalcServant*)> f = boost::bind(&CalcServant::AddInt,_1,a,b);

		//utworzenie funktora
		//mowimy mu, zeby zwracal int i ze klasa servanta to CalcServant
		DLOG(log_ << "Add() - request creation" << endl);
		MethodRequest<int,CalcServant>* request = new MethodRequest<int,CalcServant>(f,pContent);

		//bierzemy wskaznik do klasy bazowej, zeby moc go przechowac w kolejce
		Functor<CalcServant>* functor = request;

		//czysta magia
		DLOG(log_ << "Add() - pushing into AQ" << endl);
		AQ_->push(functor);

		DLOG(log_ << "Add() - returning future" << endl);
		return fut;
	}

	Future<int> DivideInt(int a, int b)
	{
		DLOG(log_ << "Add() - content creation" << endl);
		boost::shared_ptr<FutureContent> pContent(new FutureContent());
		DLOG(log_ << "Add() - future creation" << endl);
		Future<int> fut(pContent);
		DLOG(log_ << "Add() - binding" << endl);
		boost::function<int(CalcServant*)> f = boost::bind(&CalcServant::DivideInt,_1,a,b);
		DLOG(log_ << "Add() - request creation" << endl);
		MethodRequest<int,CalcServant>* request = new MethodRequest<int,CalcServant>(f,pContent);
		Functor<CalcServant>* functor = request;
		DLOG(log_ << "Add() - pushing into AQ" << endl);
		AQ_->push(functor);
		DLOG(log_ << "Add() - returning future" << endl);
		return fut;
	}

	Future<int> ReallyFrickinLongAddInt(int a, int b)
	{
		DLOG(log_ << "Add() - content creation" << endl);
		boost::shared_ptr<FutureContent> pContent(new FutureContent());
		DLOG(log_ << "Add() - future creation" << endl);
		Future<int> fut(pContent);
		DLOG(log_ << "Add() - binding" << endl);
		boost::function<int(CalcServant*)> f = boost::bind(&CalcServant::ReallyFrickinLongAddInt,_1,a,b);
		DLOG(log_ << "Add() - request creation" << endl);
		MethodRequest<int,CalcServant>* request = new MethodRequest<int,CalcServant>(f,pContent);
		Functor<CalcServant>* functor = request;
		DLOG(log_ << "Add() - pushing into AQ" << endl);
		AQ_->push(functor);
		DLOG(log_ << "Add() - returning future" << endl);
		return fut;
	}
};

#endif