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
		boost::shared_ptr<FutureContent> pContent(new FutureContent());

		//future do contentu
		Future<int> fut(pContent);

		//zbindowanie odpowiedniej metody, pozostawienie argumentu na wskaznik do konkretnego servanta
		boost::function<int(CalcServant*)> f = boost::bind(CalcServant::AddInt,_1,a,b);

		//utworzenie funktora
		//mowimy mu, zeby zwracal int i ze klasa servanta to CalcServant
		MethodRequest<int,CalcServant>* request = new MethodRequest<int,CalcServant>(f,pContent);

		//bierzemy wskaznik do klasy bazowej, zeby moc go przechowac w kolejce
		Functor<CalcServant>* functor = request;

		//czysta magia
		AQ_->push(functor);
		return fut;
	}
};

#endif