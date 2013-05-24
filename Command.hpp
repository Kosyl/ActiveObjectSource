#ifndef _COMMAND_
#define _COMMAND_

#include <boost\function.hpp>
#include "Future.hpp"

using namespace std;

class NullCommandException: public exception{};

class Functor
{
public:
	Functor(){}

	virtual void execute()=0;
};


template<typename T>
class MethodRequest:public Functor
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