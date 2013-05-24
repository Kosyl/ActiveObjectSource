#ifndef _SERVANT_
#define _SERVANT_

#include "SimpleLog.hpp"

using namespace std;

class Servant
{
public:
	Servant(){}
	virtual ~Servant(){}
};

class Calc: public Servant
{
private:
	Logger log_;
public:
	Calc():
	log_("SERVANT")
	{}

	int AddInt(int a. int b)
	{
		return a+b;
	}
	double AddDouble(double a, double b)
	{
		return a+b;
	}
};

#endif