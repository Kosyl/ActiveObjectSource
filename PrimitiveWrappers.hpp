#ifndef _PRIMITIVEWRAPPERS_
#define _PRIMITIVEWRAPPERS_

#include "SimpleLog.hpp"

using namespace std;

class Int
{
private:
	int val_;
	Logger log;
public:
	Int():
		val_(0)
	{}

	Int(int i):
		val_(i)
	{}

	Int(const int& i):
		val_(i)
	{}

	Int operator
};

#endif