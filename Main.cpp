#ifndef _MAIN_
#define _MAIN_

#include "Future.hpp"
#include "Example1.hpp"
#include "SimpleLog.hpp"
#include <boost\bind.hpp>
#include <boost\function.hpp>
#include <boost\thread.hpp>
#include "Example2_kolejka.hpp"

using namespace std;

void testFuture()
{
	DLOG(
		Logger log("MAIN");
	log << "//////////////////Test rzutowania///////////////////" << endl;
	)
}

void testSyncProxy()
{
	DLOG(Logger log("MAIN"));
	DLOG(log << "//////////////////Test proxy///////////////////" << endl);
	for(int i=0;i<3;++i)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
		DLOG(log << "//////////////////tworze proxy///////////////////" << endl);
		CalcProxy p(3);
		DLOG(log << "//////////////////kasuje proxy...///////////////////" << endl);
	}
}

void testSimpleInvoke()
{
	DLOG(Logger log("MAIN"));
	DLOG(log << "//////////////////Test invoke///////////////////" << endl);

	DLOG(log << "//////////////////tworze proxy///////////////////" << endl);
	CalcProxy p(1);
	DLOG(log << "//////////////////wolam future dodawania///////////////////" << endl);
	struct call
	{
		void operator ()(double x)
		{
			cout << "Progress listener: " << x << endl;
		}
	} callback;
	Future<int> f = p.AddInt(3,5);
	f.setFunction(callback);

	DLOG(log << "//////////////////czekam...///////////////////" << endl);
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

	DLOG(log << "//////////////////wynik///////////////////" << endl);
	DLOG(log << "//////////////////" << f.getValue() << "///////////////////" << endl);

	DLOG(log << "//////////////////wolam future dzielenia///////////////////" << endl);
}

void testException()
{
	DLOG(Logger log("MAIN"));
	DLOG(log << "//////////////////Test exception///////////////////" << endl);

	DLOG(log << "//////////////////tworze proxy///////////////////" << endl);
	CalcProxy p(1);

	DLOG(log << "//////////////////wolam future dzielenia///////////////////" << endl);
	
	Future<int> f2 = p.DivideInt(3,0);
	DLOG(log << "//////////////////czekam...///////////////////" << endl);
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

	DLOG(log << "//////////////////wynik///////////////////" << endl);
	try
	{
		f2.getValue();
	}
	catch(exception& e)
	{
		DLOG(log << "exception: " << e.what() << endl);
	}
	DLOG(log << "//////////////////kasuje proxy...///////////////////" << endl);
}

void testCancel()
{
	DLOG(Logger log("MAIN"));
	DLOG(log << "//////////////////Test cancel///////////////////" << endl);

	DLOG(log << "//////////////////tworze proxy///////////////////" << endl);
	CalcProxy p(1);
	DLOG(log << "//////////////////wolam future dlugiego dodawania///////////////////" << endl);
	
	Future<int> f = p.ReallyFrickinLongAddInt(3,5);
	DLOG(log << "//////////////////czekam...///////////////////" << endl);
	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));

	f.cancelRequest();
	DLOG(log << "//////////////////wynik///////////////////" << endl);
	try
	{
		f.getValue();
	}
	catch(exception& e)
	{
		DLOG(log << "exception: " << e.what() << endl);
	}
}

void testSharedContent()
{
	DLOG(Logger log("MAIN"));
	DLOG(log << "//////////////////Test wspoldzielonego contentu///////////////////" << endl);

	DLOG(log << "//////////////////tworze proxy///////////////////" << endl);
	CalcProxy p(1);
	DLOG(log << "//////////////////wolam future dlugiego dodawania///////////////////" << endl);
	
	Future<int> f = p.ReallyFrickinLongAddInt(3,5);
	DLOG(log << "//////////////////czekam...///////////////////" << endl);
	boost::this_thread::sleep(boost::posix_time::milliseconds(3000));

	Future<int>f2(f);
	f.cancelRequest();
	DLOG(log << "//////////////////pierwszy callback///////////////////" << endl);
	try
	{
		f.getValue();
	}
	catch(exception& e)
	{
		DLOG(log << "exception: " << e.what() << endl);
	}

	boost::this_thread::sleep(boost::posix_time::milliseconds(3000));
	f2.cancelRequest();
	try
	{
		f2.getValue();
	}
	catch(exception& e)
	{
		DLOG(log << "exception: " << e.what() << endl);
	}
}

void testSwapCallback()
{
	DLOG(Logger log("MAIN"));
	DLOG(log << "//////////////////Test swap callback///////////////////" << endl);

	DLOG(log << "//////////////////tworze proxy///////////////////" << endl);
	CalcProxy p(1);
	DLOG(log << "//////////////////wolam future dodawania///////////////////" << endl);
	struct call
	{
		void operator ()(double x)
		{
			cout << "Progress listener: " << x << endl;
		}
	} callback;
	Future<int> f = p.ReallyFrickinLongAddInt(30,50);
	f.setFunction(callback);

	DLOG(log << "//////////////////czekam...///////////////////" << endl);
	boost::this_thread::sleep(boost::posix_time::milliseconds(2500));

	DLOG(log << "//////////////////zmieniam callback///////////////////" << endl);
	struct call2
	{
		void operator ()(double x)
		{
			cout << "Nowy Wspanialy Lepszy Progress listener: " << x << endl;
		}
	} callback2;
	f.setFunction(callback2);

	DLOG(log << "//////////////////czekam...///////////////////" << endl);
	boost::this_thread::sleep(boost::posix_time::milliseconds(2500));

	DLOG(log << "//////////////////zmieniam callback///////////////////" << endl);
	struct call3
	{
		void operator ()(double x)
		{
			cout << "KRZYCZACY NOWY WSPANIALY LISTENER PROGRESSU: " << x << endl;
		}
	} callback3;
	f.setFunction(callback3);

	DLOG(log << "//////////////////blokuje...///////////////////" << endl);
	f.getValue();
}

void testManyThreads()
{
	DLOG(Logger log("MAIN"));
	DLOG(log << "//////////////////Test - >1 thread///////////////////" << endl);

	DLOG(log << "//////////////////tworze proxy///////////////////" << endl);
	CalcProxy p(3);
	DLOG(log << "//////////////////wolam future dodawania///////////////////" << endl);
	
	Future<int> f1 = p.ReallyFrickinLongAddInt(30,50);
	Future<int> f2 = p.ReallyFrickinLongAddInt(34,51);
	Future<int> f3 = p.ReallyFrickinLongAddInt(3,58);
	Future<int> f4 = p.ReallyFrickinLongAddInt(1,234);
	Future<int> f5 = p.ReallyFrickinLongAddInt(23,523);
	Future<int> f6 = p.ReallyFrickinLongAddInt(43,34);
	Future<int> f7 = p.ReallyFrickinLongAddInt(100,34);
	Future<int> f8 = p.ReallyFrickinLongAddInt(2,34);

	DLOG(log << "//////////////////czekamy...///////////////////" << endl);
	
	DLOG(log << "f1: " << f1.getValue() << endl);
	DLOG(log << "f2: " << f2.getValue() << endl);
	DLOG(log << "f3: " << f3.getValue() << endl);
	DLOG(log << "f4: " << f4.getValue() << endl);
	DLOG(log << "f5: " << f5.getValue() << endl);
	DLOG(log << "f6: " << f6.getValue() << endl);
	DLOG(log << "f7: " << f7.getValue() << endl);
	DLOG(log << "f8: " << f8.getValue() << endl);
	
}

void testSingletonServant()
{
	DLOG(Logger log("MAIN"));
	DLOG(log << "//////////////////Test servant-singleton///////////////////" << endl);

	DLOG(log << "//////////////////tworze proxy///////////////////" << endl);
	SyncCalcProxy p(3);
	DLOG(log << "//////////////////wolam future dodawania///////////////////" << endl);
	
	Future<int> f1 = p.SlowAddInt(30,50);
	Future<int> f2 = p.SlowAddInt(34,51);
	Future<int> f3 = p.SlowAddInt(3,58);
	Future<int> f4 = p.SlowAddInt(1,234);

	DLOG(log << "//////////////////czekamy...///////////////////" << endl);
	
	DLOG(log << "f1: " << f1.getValue() << endl);
	DLOG(log << "f2: " << f2.getValue() << endl);
	DLOG(log << "f3: " << f3.getValue() << endl);
	DLOG(log << "f4: " << f4.getValue() << endl);
}

void testPersistantFuture()
{
	DLOG(Logger log("MAIN"));
	DLOG(log << "//////////////////Test dlugotrwalego future///////////////////" << endl);

	DLOG(log << "//////////////////tworze proxy///////////////////" << endl);
	CalcProxy* p = new CalcProxy(1);
	DLOG(log << "//////////////////wolam future dlugiego dodawania///////////////////" << endl);
	
	Future<int> f = p->ReallyFrickinLongAddInt(3,5);

	DLOG(log << "//////////////////czekam na wartosc w trybie blokujacym///////////////////" << endl);
	try
	{
		DLOG(log << "//////////////////" << f.getValue() << "///////////////////" << endl);
	}
	catch(exception& e)
	{
		DLOG(log << "exception: " << e.what() << endl);
	}

	boost::this_thread::sleep(boost::posix_time::milliseconds(2000));

	DLOG(log << "//////////////////kasuje proxy///////////////////" << endl);
	delete p;

	boost::this_thread::sleep(boost::posix_time::milliseconds(3000));

	DLOG(log << "//////////////////Ponowna proba odczytania wartosci///////////////////" << endl);
	try
	{
		DLOG(log << "//////////////////" << f.getValue() << "///////////////////" << endl);
	}
	catch(exception& e)
	{
		DLOG(log << "exception: " << e.what() << endl);
	}

	DLOG(log << "//////////////////cancel powinien skasowac content//////////////////" << endl);
	f.cancelRequest();
	try
	{
		DLOG(log << "//////////////////" << f.getValue() << "///////////////////" << endl);
	}
	catch(exception& e)
	{
		DLOG(log << "exception: " << e.what() << endl);
	}
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
}

void testManyMethods()
{
	DLOG(Logger log("MAIN"));
	DLOG(log << "//////////////////Test wielu metod///////////////////" << endl);

	DLOG(log << "//////////////////tworze proxy///////////////////" << endl);
	CalcProxy p(2);
	DLOG(log << "//////////////////wolam future dodawania///////////////////" << endl);
	
	Future<int> f1 = p.AddInt(30,50);
	Future<int> f2 = p.SlowAddInt(34,51);
	Future<int> f3 = p.ReallyFrickinLongAddInt(3,58);
	Future<int> f4 = p.DivideInt(234,2);
	Future<double> f5 = p.DivideDouble(12312.23,5434.99);

	DLOG(log << "//////////////////czekamy...///////////////////" << endl);
	
	DLOG(log << "f1: " << f1.getValue() << endl);
	DLOG(log << "f2: " << f2.getValue() << endl);
	DLOG(log << "f3: " << f3.getValue() << endl);
	DLOG(log << "f4: " << f4.getValue() << endl);
	DLOG(log << "f5: " << f5.getValue() << endl);
}

void testGuard()
{
	DLOG(Logger log("MAIN"));
	DLOG(log << "//////////////////Test guarda///////////////////" << endl);

	DLOG(log << "//////////////////tworze proxy///////////////////" << endl);
	QueueProxy p(1);
	DLOG(log << "//////////////////wolam future dodawania///////////////////" << endl);
	
	Future<void> f1 = p.Put("msg1");
	Future<void> f2 = p.Put("msg2");
	Future<void> f3 = p.Put("msg3");
	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
	DLOG(log << "//////////////////Wyjmowanie z kolejki///////////////////" << endl);
	Future<std::string> f4 = p.Get();
	Future<std::string> f5 = p.Get();
	Future<std::string> f6 = p.Get();

	DLOG(log << "//////////////////czekamy...///////////////////" << endl);
	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
	DLOG(log << "//////////////////getValue...///////////////////" << endl);
	DLOG(log << "f4: " << f4.getValue() << endl);
	DLOG(log << "f5: " << f5.getValue() << endl);
	DLOG(log << "f6: " << f6.getValue() << endl);
}

void testVoidInvokes()
{
	DLOG(Logger log("MAIN"));
	DLOG(log << "//////////////////Test metod void, zobaczymy, czy siê wykonaj¹///////////////////" << endl);

	DLOG(log << "//////////////////tworze proxy///////////////////" << endl);
	QueueProxy p(1);
	DLOG(log << "//////////////////wolam metody dodawania///////////////////" << endl);
	
	p.Put("msg1");
	p.Put("msg2");
	p.Put("msg3");
	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
	DLOG(log << "//////////////////Wyjmowanie z kolejki///////////////////" << endl);
	Future<std::string> f4 = p.Get();
	Future<std::string> f5 = p.Get();
	Future<std::string> f6 = p.Get();

	DLOG(log << "//////////////////czekamy...///////////////////" << endl);
	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
	DLOG(log << "//////////////////getValue...///////////////////" << endl);
	DLOG(log << "f4: " << f4.getValue() << endl);
	DLOG(log << "f5: " << f5.getValue() << endl);
	DLOG(log << "f6: " << f6.getValue() << endl);
}

void testGuardMultipleThreads()
{
	DLOG(Logger log("MAIN"));
	DLOG(log << "//////////////////Test guarda///////////////////" << endl);

	DLOG(log << "//////////////////tworze proxy///////////////////" << endl);
	SyncQueueProxy p(3);
	DLOG(log << "//////////////////wolam future dodawania///////////////////" << endl);
	
	Future<void> f1 = p.Put("msg1");
	Future<void> f2 = p.Put("msg2");
	Future<void> f3 = p.Put("msg3");
	Future<void> f4 = p.Put("msg4");
	Future<void> f5 = p.Put("msg5");
	Future<void> f6 = p.Put("msg6");
	Future<void> f7 = p.Put("msg7");
	Future<void> f8 = p.Put("msg8");
	Future<void> f9 = p.Put("msg9");
	DLOG(log << "//////////////////czekamy...///////////////////" << endl);
	boost::this_thread::sleep(boost::posix_time::milliseconds(10000));
	DLOG(log << "//////////////////Wyjmowanie z kolejki///////////////////" << endl);
	for(int i=0;i<9;++i)
	{
		Future<std::string> f_ans = p.Get();
		DLOG(log << "f_ans: " << f_ans.getValue() << endl);
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	}

	DLOG(log << "//////////////////kasujemy wsio...///////////////////" << endl);
}

void testRefreshPeriod()
{
	DLOG(Logger log("MAIN"));
	DLOG(log << "//////////////////Test refresh period///////////////////" << endl);

	DLOG(log << "//////////////////tworze proxy///////////////////" << endl);
	SyncQueueProxy p(3,2000L);

	DLOG(log << "//////////////////wolam future dodawania///////////////////" << endl);
	
	Future<void> f1 = p.Put("msg1");
	Future<void> f2 = p.Put("msg2");
	Future<void> f3 = p.Put("msg3");
	Future<void> f4 = p.Put("msg4");
	Future<void> f5 = p.Put("msg5");
	Future<void> f6 = p.Put("msg6");
	Future<void> f7 = p.Put("msg7");
	Future<void> f8 = p.Put("msg8");
	Future<void> f9 = p.Put("msg9");
	Future<void> f10 = p.Put("msg10");
	DLOG(log << "//////////////////czekamy...///////////////////" << endl);
	boost::this_thread::sleep(boost::posix_time::milliseconds(4000));
	DLOG(log << "//////////////////Wyjmowanie z kolejki///////////////////" << endl);
	for(int i=0;i<10;++i)
	{
		Future<std::string> f_ans = p.Get();
		DLOG(log << "f_ans: " << f_ans.getValue() << endl);
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	}

	DLOG(log << "//////////////////kasujemy wsio...///////////////////" << endl);
}

int main(int argc, char* argv[])
{	
	//testSyncProxy();

	//testSimpleInvoke();

	//testException();

	//testCancel();

	//testSharedContent();

	//testSwapCallback();

	//testManyThreads();

	//testSingletonServant();

	//testPersistantFuture();
	
	//testManyMethods();

	//testGuard();

	//testVoidInvokes();

	//testGuardMultipleThreads();

	testRefreshPeriod();

	system("PAUSE");

	return EXIT_SUCCESS;
}

#endif