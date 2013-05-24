#ifndef _MAIN_
#define _MAIN_

//#include "Command.hpp"
#include "Future.hpp"
#include "Promise.hpp"
#include "SimpleLog.hpp"
#include <boost\bind.hpp>
#include <boost\function.hpp>

using namespace std;

class X
{
public:
	int a;
	Logger log;

	X():
		log("X")
	{}

	int fun1(int a, int b)
	{
		log << "fun1" << endl;
		a=a+b;
		return a;
	}

	double fun2(double a, double b)
	{
		log << "fun2" << endl;
		return a-b;
	}
};

void callback(double progress)
{
	cout << "CALLBACK: " << progress << endl;
}

void testFuture()
{
	Logger log("MAIN");
	log << "//////////////////Test rzutowania///////////////////" << endl;
	log << "Promise creation" << endl;
	Promise promise;
	log << "Future creation" << endl;
	Future<int> future = promise.getFuture<int>();
	future.setFunction(boost::bind(callback,_1));
	log << "Check" << endl;
	log << "HasException: " << future.hasException() << endl;
	if(future.hasException())log << "Exception: " << future.getException().what() << endl;
	log << "Progress: " << future.getProgress() << endl;
	log << "IsDone: " << future.isDone() << endl;
	log << "Progress set (45%)" << endl;
	promise.setProgress(0.45);
	log << "Second check" << endl;
	log << "HasException: " << future.hasException() << endl;
	if(future.hasException())log << "Exception: " << future.getException().what() << endl;
	log << "Progress: " << future.getProgress() << endl;
	log << "IsDone: " << future.isDone() << endl;
	log << "Set value" << endl;
	promise.setProgress(0.65);
	promise.setProgress(0.89);
	promise.setValue<int>(56);
	log << "Third check" << endl;
	log << "HasException: " << future.hasException() << endl;
	if(future.hasException())log << "Exception: " << future.getException().what() << endl;
	log << "Progress: " << future.getProgress() << endl;
	log << "IsDone: " << future.isDone() << endl;
	log << "getValue: " << future.getValue() << endl;
}

int main(int argc, char* argv[])
{	
	//Proxy proxy;
	testFuture();

	//boost::shared_ptr<Future<int>>  re1 = proxy.quickSchedule<int>(boost::bind(fun1,4,12));
	//boost::shared_ptr<Future<double>>  re2 = proxy.quickSchedule<double>(boost::bind(fun2,4.34,1.12));

	//boost::shared_ptr<Future<double>>  re3 = proxy.quickSchedule<double>(boost::bind(fun1,3,4));

	//proxy.quickExecute();

	////Future<int>* res1 = proxy.quickSchedule<double>(boost::bind(fun1,4,12));
	////Future<double>* res1 = proxy.quickSchedule<int>(boost::bind(fun1,4,12));

	//struct meth: public Command<string>
	//{
	//	string myFun(string a, string b)
	//	{
	//		return a+b;
	//	}
	//	virtual void execute()
	//	{

	//	}

	//}progressCmd;

	////testowa zmiana zeby zobaczyc czy git wylapie

	//boost::shared_ptr<Future<int>> res1 = proxy.quickSchedule<int>(boost::bind(fun1,4,12));
	//boost::shared_ptr<Future<double>> res2 = proxy.quickSchedule<double>(boost::bind(fun2,4.34,1.12));

	//cout << "res1 ready? " << res1->isDone() << endl;
	//cout << "res2 ready? " << res2->isDone() << endl;
	//cout << "execution" << endl;
	//proxy.quickExecute();

	//cout << "res1 ready? " << res1->isDone() << endl;
	//cout << "res2 ready? " << res2->isDone() << endl;
	//cout << "res1 value " << res1->getValue() << endl;
	//cout << "res2 value " << res2->getValue() << endl;

	system("PAUSE");

	return EXIT_SUCCESS;
}

#endif