/**
* @file Example1.hpp
* @author Michal Kosyl
* @author Marta Kuzak
* @details Active Object implementation.
* @details Sample implementation of the Active Object pattern
*/
#ifndef _EXAMPLE1_
#define _EXAMPLE1_

#include "SimpleLog.hpp"
#include "FutureContentCreator.hpp"
#include "Proxy.hpp"
#include "Future.hpp"
#include "MethodRequest.hpp"
#include <boost/function.hpp>
#include <boost/thread.hpp>

using namespace std;

using namespace ActiveObject;

/**
* @brief Simple caclulator class
*/
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
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));
		setProgress(0.8);
		return a+b;
	}

	int ReallyLongAddInt(int a, int b)
	{
		for(double i=0;i<1.0;i+=0.1)
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
			setProgress(i);
			if(isCancelled()) 
				throw RequestCancelledException();
		}

		return a+b;
	}

	int SlowAddInt(int a, int b)
	{
		for(double i=0;i<1.0;i+=0.5)
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(1500));
			setProgress(i);
			if(isCancelled()) 
				throw RequestCancelledException();
		}

		return a+b;
	}

	int DivideInt(int a, int b)
	{
		setProgress(0.2);
		if (b == 0)
			throw std::overflow_error("Divide by zero exception");
		setProgress(0.8);
		return a/b;
	}

	double DivideDouble(double a, double b)
	{
		setProgress(0.2);
		if (b == 0.0)
			throw std::overflow_error("Divide by zero exception");
		setProgress(0.8);
		return a/b;
	}
};

/**
* @brief Simple caclulator class. Every method is synchronized
*/
class SyncCalcServant: public FutureContentCreator
{
private:
	boost::mutex mutex_;

public:

	SyncCalcServant()
	{}

	int AddInt(int a, int b)
	{
		boost::mutex::scoped_lock lock(mutex_);
		setProgress(0.2);
		setProgress(0.8);
		return a+b;
	}

	int ReallyLongAddInt(int a, int b)
	{
		boost::mutex::scoped_lock lock(mutex_);
		for(double i=0;i<1.0;i+=0.1)
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
			setProgress(i);
			if(isCancelled()) 
				throw RequestCancelledException();
		}

		return a+b;
	}

	int SlowAddInt(int a, int b)
	{
		boost::mutex::scoped_lock lock(mutex_);
		for(double i=0;i<1.0;i+=0.5)
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(1500));
			setProgress(i);
			if(isCancelled()) 
				throw RequestCancelledException();
		}

		return a+b;
	}

	int DivideInt(int a, int b)
	{
		boost::mutex::scoped_lock lock(mutex_);
		setProgress(0.2);
		if (b == 0)
			throw std::overflow_error("Divide by zero exception");
		setProgress(0.8);
		return a/b;
	}

};

/**
* @brief Proxy to the simple caclulator class
* the calling template looks like this:
* Future<ReturnType> MethodName(paramType1 p1, paramType2 p2...)
* {
*     return enqueue<ReturnType>(boost::bind(&ServantClass::ServantMethod,_1,p1,p2,...));
* }
*/
class CalcProxy: public Proxy<CalcServant,ServantFactoryCreator>
{

public:

	CalcProxy(int numThreads=1):
		Proxy(numThreads)
	{}

	Future<int> AddInt(int a, int b)
	{
		return enqueue<int>(boost::bind(&CalcServant::AddInt,_1,a,b));
	}

	Future<int> DivideInt(int a, int b)
	{
		return enqueue<int>(boost::bind(&CalcServant::DivideInt,_1,a,b));
	}

	Future<int> ReallyLongAddInt(int a, int b)
	{
		return enqueue<int>(boost::bind(&CalcServant::ReallyLongAddInt,_1,a,b));
	}

	Future<int> SlowAddInt(int a, int b)
	{
		return enqueue<int>(boost::bind(&CalcServant::SlowAddInt,_1,a,b));
	}

	Future<double> DivideDouble(double a, double b)
	{
		return enqueue<double>(boost::bind(&CalcServant::DivideDouble,_1,a,b));
	}
};

/**
* @brief Proxy to the simple synchronized caclulator class (every scheduler access the same servant instance)
*/
class SyncCalcProxy: public Proxy<SyncCalcServant,ServantSingletonCreator>
{

public:

	SyncCalcProxy(int numThreads=1):
		Proxy(numThreads)
	{}

	Future<int> AddInt(int a, int b)
	{
		return enqueue<int>(boost::bind(&SyncCalcServant::AddInt,_1,a,b));
	}

	Future<int> DivideInt(int a, int b)
	{
		return enqueue<int>(boost::bind(&SyncCalcServant::DivideInt,_1,a,b));
	}

	Future<int> ReallyFrickinLongAddInt(int a, int b)
	{
		return enqueue<int>(boost::bind(&SyncCalcServant::ReallyLongAddInt,_1,a,b));
	}

	Future<int> SlowAddInt(int a, int b)
	{
		return enqueue<int>(boost::bind(&SyncCalcServant::SlowAddInt,_1,a,b));
	}
};

#endif
