#ifndef _MAIN_
#define _MAIN_

//#include "Command.hpp"
#include "Future.hpp"
#include "Example1.hpp"
#include "SimpleLog.hpp"
#include <boost\bind.hpp>
#include <boost\function.hpp>

using namespace std;

class X
{
public:
	int a;
	Logger log;

	X(int b=0):
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

	//ten test juz nie jest aktualny chyba:P
	//bedzie trzeba przerobic

	/*log << "Promise creation" << endl;
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
	log << "Future copy" << endl;
	Future<int> future2 = promise.getFuture<int>();
	Future<int> future3= future2;
	log << "Future: = operator" << endl;
	future3= future;*/
}

void testProxy()
{
	//TODO wszystko
}

int main(int argc, char* argv[])
{	
	//testFuture();

	testProxy();
	
	system("PAUSE");

	return EXIT_SUCCESS;
}

#endif