#ifndef _PRIMITIVEWRAPPERS_
#define _PRIMITIVEWRAPPERS_

#include "SimpleLog.hpp"

using namespace std;

class Int
{
private:
	int val_;
	Logger log_;
public:
	Int():
		log_("INT"),
		val_(0)
	{}

	Int(int i):
		log_("INT"),
		val_(i)
	{}

	Int(const int& i):
		log_("INT"),
		val_(i)
	{}


};

#endif