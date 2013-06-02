#ifndef _EXAMPLE2_
#define _EXAMPLE2_

#include "SimpleLog.hpp"
#include "FutureContentCreator.hpp"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include "Proxy.hpp"
#include "Future.hpp"
#include "MethodRequest.hpp"
#include <queue>

using namespace std;
using namespace ActiveObject;
//kolejka 5ciu komunikatow
class QueueServant: public FutureContentCreator
{

public:

	QueueServant()
	{}

	QueueServant(const QueueServant& rhs)
	{}

	void put(std::string s)
	{
		DLOG(log_ << "put" << endl);
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
		queue_.push(s);
	}

	std::string get()
	{
		DLOG(log_ << "get" << endl);
		boost::this_thread::sleep(boost::posix_time::milliseconds(1500));
		std::string tmp = queue_.front();
		queue_.pop();
		return tmp;
	}

	bool isFull()
	{
		DLOG(log_ << "isFull (" << (queue_.size()>=2) << ")" << endl);
		return queue_.size()>=2;
	}

	bool isEmpty()
	{
		DLOG(log_ << "isEmpty (" << (queue_.empty()) << ")" << endl);
		return queue_.empty();
	}

private:

	std::queue <std::string> queue_; 
};

class SyncQueueServant: public FutureContentCreator
{

public:

	SyncQueueServant()
	{}

	SyncQueueServant(const SyncQueueServant& rhs)
	{}

	void put(std::string s)
	{
		boost::mutex::scoped_lock lock(mutex_);
		DLOG(log_ << "put" << endl);
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
		queue_.push(s);
	}

	std::string get()
	{
		boost::mutex::scoped_lock lock(mutex_);
		DLOG(log_ << "get" << endl);
		boost::this_thread::sleep(boost::posix_time::milliseconds(1500));
		std::string tmp = queue_.front();
		queue_.pop();
		return tmp;
	}

	bool isFull()
	{
		boost::mutex::scoped_lock lock(mutex_);
		DLOG(log_ << "isFull (" << (queue_.size()>=2) << ")" << endl);
		return queue_.size()>=2;
	}

	bool isEmpty()
	{
		boost::mutex::scoped_lock lock(mutex_);
		DLOG(log_ << "isEmpty (" << (queue_.empty()) << ")" << endl);
		return queue_.empty();
	}

private:

	std::queue <std::string> queue_; 
	boost::mutex mutex_;
};


//proxy do servanta
//mowimy, jaki typ servanta i jak jest generowany dla kazdego schedulera
class QueueProxy: public Proxy<QueueServant,ServantFactoryCreator>
{

public:

	QueueProxy(int numThreads=1):
		Proxy(numThreads)
	{}

	

	//tyle kodu bedzie potrzeba na obsluge zadania jednej funkcji
	//bardzo mozliwe ze da sie to zautomatyzowac
	Future<void> Put(std::string s)
	{
		return enqueue<void>(boost::bind(&QueueServant::put,_1,s), boost::bind(&QueueServant::isFull,_1));
	}

	Future<std::string> Get()
	{
		return enqueue<std::string>(boost::bind(&QueueServant::get,_1), boost::bind(&QueueServant::isEmpty,_1));
	}
};

class SyncQueueProxy: public Proxy<QueueServant,ServantSingletonCreator>
{

public:

	SyncQueueProxy(int numThreads=1):
		Proxy(numThreads)
	{}

	SyncQueueProxy(int numThreads, unsigned long refreshPeriod):
		Proxy(numThreads,refreshPeriod)
	{}

	//tyle kodu bedzie potrzeba na obsluge zadania jednej funkcji
	//bardzo mozliwe ze da sie to zautomatyzowac
	Future<void> Put(std::string s)
	{
		return enqueue<void>(boost::bind(&QueueServant::put,_1,s), boost::bind(&QueueServant::isFull,_1));
	}

	Future<std::string> Get()
	{
		return enqueue<std::string>(boost::bind(&QueueServant::get,_1), boost::bind(&QueueServant::isEmpty,_1));
	}
};


#endif
