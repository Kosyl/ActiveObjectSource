#ifndef _COMMAND_
#define _COMMAND_

#include <boost\function.hpp>
#include "Future.hpp"

using namespace std;

class Proxy;

template<typename T>
class Command
{
	friend class Proxy;
	boost::function<T()> cmd_;

private:
	Promise<T>* promise_;

	void setPromise(Promise<T>* promise)
	{
		promise_=promise;
	}

public:
	Command():
		promise_()
	{};

	virtual void execute()=0;

protected:
	void setProgress(double progress)
	{
		cout << "COMMAND: SetProgress : " << progress << endl;
		promise_->setProgress(progress);
	}

	void setException(string e)
	{
		cout << "COMMAND: Exception : " << e << endl;
		promise_->
	}
};

class NullCommandException: public exception{};

class Funktor
{
public:
	Funktor(){}

	virtual void execute()=0;
};


template<typename T>
class MethodRequest:public Funktor
{
private:
	boost::function<T()> command_;
	Promise<T>* promise_;
public:
	MethodRequest(){};
	MethodRequest(boost::function<T()> f, Promise<T>* prom):
		command_(f),
		promise_(prom)
	{}

	void execute()
	{
		if(command_)
		{
			promise_->setValue(command_());
		}
		else
		{
			throw new NullCommandException;
		}
	}

	template<typename FuncType>
	void setFunction(FuncType fun)
	{
		command_=fun;
	}

	bool isReady() const
	{
		return command_;
	}
};

#endif