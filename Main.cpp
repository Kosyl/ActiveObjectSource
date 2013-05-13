#ifndef _MAIN_
#define _MAIN_

#include "Command.hpp"
#include "Future.hpp"
#include "Proxy.hpp"
#include "SimpleLog.hpp"
#include<boost\bind.hpp>

using namespace std;

int fun1(int a, int b)
{
	cout << "fun1" << endl;
	return a+b;
}

double fun2(double a, double b)
{
	cout << "fun2" << endl;
	return a-b;
}

int main(int argc, char* argv[])
{	
	Proxy proxy;
	Logger log("MAIN");
	log << "//////////////////Test rzutowania///////////////////";

	boost::shared_ptr<Future<int>>  re1 = proxy.quickSchedule<int>(boost::bind(fun1,4,12));
	boost::shared_ptr<Future<double>>  re2 = proxy.quickSchedule<double>(boost::bind(fun2,4.34,1.12));

	boost::shared_ptr<Future<double>>  re3 = proxy.quickSchedule<double>(boost::bind(fun1,3,4));

	proxy.quickExecute();

	//Future<int>* res1 = proxy.quickSchedule<double>(boost::bind(fun1,4,12));
	//Future<double>* res1 = proxy.quickSchedule<int>(boost::bind(fun1,4,12));

	struct meth: public Command<string>
	{
		string myFun(string a, string b)
		{
			return a+b;
		}
		virtual void execute()
		{

		}

	}progressCmd;

	//testowa zmiana zeby zobaczyc czy git wylapie

	boost::shared_ptr<Future<int>> res1 = proxy.quickSchedule<int>(boost::bind(fun1,4,12));
	boost::shared_ptr<Future<double>> res2 = proxy.quickSchedule<double>(boost::bind(fun2,4.34,1.12));

	cout << "res1 ready? " << res1->isDone() << endl;
	cout << "res2 ready? " << res2->isDone() << endl;
	cout << "execution" << endl;
	proxy.quickExecute();

	cout << "res1 ready? " << res1->isDone() << endl;
	cout << "res2 ready? " << res2->isDone() << endl;
	cout << "res1 value " << res1->getValue() << endl;
	cout << "res2 value " << res2->getValue() << endl;

	system("PAUSE");

	return EXIT_SUCCESS;
}

#endif