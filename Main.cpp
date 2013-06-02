#ifndef _MAIN_
#define _MAIN_

#define BOOST_TEST_MAIN
#include <string>
#include <boost/function.hpp>
#include "Future.hpp"
#include "Example1.hpp"
#include "SimpleLog.hpp"
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include "Example2_kolejka.hpp"
#include <boost/test/unit_test.hpp>
#include <boost/any.hpp>

#define CHECK(x) {bool tmp=x;log.lock();BOOST_CHECK_IMPL( (tmp), BOOST_TEST_STRINGIZE( x ), CHECK, CHECK_PRED );log.unlock();}

using namespace std;
using namespace ActiveObject;

void testFuture()
{
	Logger log("MAIN");
	log << "//////////////////Test rzutowania///////////////////" << endl;

}

void testSyncProxy()
{
	Logger log("MAIN",3);
	log << "//////////////////Test proxy///////////////////" << endl;
	for(int i=0;i<3;++i)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
		log << "//////////////////tworze proxy///////////////////" << endl;
		CalcProxy p(3);
		log << "//////////////////kasuje proxy...///////////////////" << endl;
	}
}

void testSimpleInvoke()
{
	Logger log("MAIN");
	log << "//////////////////Test invoke///////////////////" << endl;

	log << "//////////////////tworze proxy///////////////////" << endl;
	//podstawowe komunikaty
	BOOST_WARN(3==2); //pojawia sie warning (gdzie? na pewno nie na konsoli), ale test dalej trwa i nie konczy sie negatywnie
	BOOST_CHECK(3==4); //wyrzuca blad na ekranie, test trwa dalej i konczy sie negatywnie
	//BOOST_REQUIRE(1==2); //wyrzuca blad i konczy test, ktory konczy sie negatywnie
	CalcProxy p(1);
	log << "//////////////////wolam future dodawania///////////////////" << endl;
	struct call
	{
		void operator ()(double x)
		{
			cout << "Progress listener: " << x << endl;
		}
	} callback;
	Future<int> f = p.AddInt(3,5);
	f.setFunction(callback);

	log << "//////////////////czekam...///////////////////" << endl;
	CHECK(f.isDone()==true);
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	CHECK(f.hasException()==true);
	log << "//////////////////wynik///////////////////" << endl;
	log << "//////////////////" << f.getValue() << "///////////////////" << endl;
	BOOST_CHECK(f.getValue()==7);
	log << "//////////////////wolam future dzielenia///////////////////" << endl;
}

void testException()
{
	Logger log("MAIN");
	log << "//////////////////Test exception///////////////////" << endl;

	log << "//////////////////tworze proxy///////////////////" << endl;
	CalcProxy p(1);

	log << "//////////////////wolam future dzielenia///////////////////" << endl;
	
	Future<int> f2 = p.DivideInt(3,0);
	log << "//////////////////czekam...///////////////////" << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

	log << "//////////////////wynik///////////////////" << endl;
	try
	{
		f2.getValue();
	}
	catch(exception& e)
	{
		log << "exception: " << e.what() << endl;
	}
	log << "//////////////////kasuje proxy...///////////////////" << endl;
}

void testCancel()
{
	Logger log("MAIN");
	log << "//////////////////Test cancel///////////////////" << endl;

	log << "//////////////////tworze proxy///////////////////" << endl;
	CalcProxy p(1);
	log << "//////////////////wolam future dlugiego dodawania///////////////////" << endl;
	
	Future<int> f = p.ReallyFrickinLongAddInt(3,5);
	log << "//////////////////czekam...///////////////////" << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));

	f.cancelRequest();
	log << "//////////////////wynik///////////////////" << endl;
	try
	{
		f.getValue();
	}
	catch(exception& e)
	{
		log << "exception: " << e.what() << endl;
	}
}

void testSharedContent()
{
	Logger log("MAIN");
	log << "//////////////////Test wspoldzielonego contentu///////////////////" << endl;

	log << "//////////////////tworze proxy///////////////////" << endl;
	CalcProxy p(1);
	log << "//////////////////wolam future dlugiego dodawania///////////////////" << endl;
	
	Future<int> f = p.ReallyFrickinLongAddInt(3,5);
	log << "//////////////////czekam...///////////////////" << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(3000));

	Future<int>f2(f);
	f.cancelRequest();
	log << "//////////////////pierwszy callback///////////////////" << endl;
	try
	{
		f.getValue();
	}
	catch(exception& e)
	{
		log << "exception: " << e.what() << endl;
	}

	boost::this_thread::sleep(boost::posix_time::milliseconds(3000));
	f2.cancelRequest();
	try
	{
		f2.getValue();
	}
	catch(exception& e)
	{
		log << "exception: " << e.what() << endl;
	}
}

void testSwapCallback()
{
	Logger log("MAIN");
	log << "//////////////////Test swap callback///////////////////" << endl;

	log << "//////////////////tworze proxy///////////////////" << endl;
	CalcProxy p(1);
	log << "//////////////////wolam future dodawania///////////////////" << endl;
	struct call
	{
		void operator ()(double x)
		{
			cout << "Progress listener: " << x << endl;
		}
	} callback;
	Future<int> f = p.ReallyFrickinLongAddInt(30,50);
	f.setFunction(callback);

	log << "//////////////////czekam...///////////////////" << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(2500));

	log << "//////////////////zmieniam callback///////////////////" << endl;
	struct call2
	{
		void operator ()(double x)
		{
			cout << "Nowy Wspanialy Lepszy Progress listener: " << x << endl;
		}
	} callback2;
	f.setFunction(callback2);

	log << "//////////////////czekam...///////////////////" << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(2500));

	log << "//////////////////zmieniam callback///////////////////" << endl;
	struct call3
	{
		void operator ()(double x)
		{
			cout << "KRZYCZACY NOWY WSPANIALY LISTENER PROGRESSU: " << x << endl;
		}
	} callback3;
	f.setFunction(callback3);

	log << "//////////////////blokuje...///////////////////" << endl;
	f.getValue();
}

void testManyThreads()
{
	Logger log("MAIN");
	log << "//////////////////Test - >1 thread///////////////////" << endl;

	log << "//////////////////tworze proxy///////////////////" << endl;
	CalcProxy p(3);
	log << "//////////////////wolam future dodawania///////////////////" << endl;
	
	Future<int> f1 = p.ReallyFrickinLongAddInt(30,50);
	Future<int> f2 = p.ReallyFrickinLongAddInt(34,51);
	Future<int> f3 = p.ReallyFrickinLongAddInt(3,58);
	Future<int> f4 = p.ReallyFrickinLongAddInt(1,234);
	Future<int> f5 = p.ReallyFrickinLongAddInt(23,523);
	Future<int> f6 = p.ReallyFrickinLongAddInt(43,34);
	Future<int> f7 = p.ReallyFrickinLongAddInt(100,34);
	Future<int> f8 = p.ReallyFrickinLongAddInt(2,34);

	log << "//////////////////czekamy...///////////////////" << endl;
	
	log << "f1: " << f1.getValue() << endl;
	log << "f2: " << f2.getValue() << endl;
	log << "f3: " << f3.getValue() << endl;
	log << "f4: " << f4.getValue() << endl;
	log << "f5: " << f5.getValue() << endl;
	log << "f6: " << f6.getValue() << endl;
	log << "f7: " << f7.getValue() << endl;
	log << "f8: " << f8.getValue() << endl;
	
}

void testSingletonServant()
{
	Logger log("MAIN");
	log << "//////////////////Test servant-singleton///////////////////" << endl;

	log << "//////////////////tworze proxy///////////////////" << endl;
	SyncCalcProxy p(3);
	log << "//////////////////wolam future dodawania///////////////////" << endl;
	
	Future<int> f1 = p.SlowAddInt(30,50);
	Future<int> f2 = p.SlowAddInt(34,51);
	Future<int> f3 = p.SlowAddInt(3,58);
	Future<int> f4 = p.SlowAddInt(1,234);

	log << "//////////////////czekamy...///////////////////" << endl;
	
	log << "f1: " << f1.getValue() << endl;
	log << "f2: " << f2.getValue() << endl;
	log << "f3: " << f3.getValue() << endl;
	log << "f4: " << f4.getValue() << endl;
}

void testPersistantFuture()
{
	Logger log("MAIN");
	log << "//////////////////Test dlugotrwalego future///////////////////" << endl;

	log << "//////////////////tworze proxy///////////////////" << endl;
	CalcProxy* p = new CalcProxy(1);
	log << "//////////////////wolam future dlugiego dodawania///////////////////" << endl;
	
	Future<int> f = p->ReallyFrickinLongAddInt(3,5);

	log << "//////////////////czekam na wartosc w trybie blokujacym///////////////////" << endl;
	try
	{
		log << "//////////////////" << f.getValue() << "///////////////////" << endl;
	}
	catch(exception& e)
	{
		log << "exception: " << e.what() << endl;
	}

	boost::this_thread::sleep(boost::posix_time::milliseconds(2000));

	log << "//////////////////kasuje proxy///////////////////" << endl;
	delete p;

	boost::this_thread::sleep(boost::posix_time::milliseconds(3000));

	log << "//////////////////Ponowna proba odczytania wartosci///////////////////" << endl;
	try
	{
		log << "//////////////////" << f.getValue() << "///////////////////" << endl;
	}
	catch(exception& e)
	{
		log << "exception: " << e.what() << endl;
	}

	log << "//////////////////cancel powinien skasowac content//////////////////" << endl;
	f.cancelRequest();
	try
	{
		log << "//////////////////" << f.getValue() << "///////////////////" << endl;
	}
	catch(exception& e)
	{
		log << "exception: " << e.what() << endl;
	}
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
}

void testManyMethods()
{
	Logger log("MAIN");
	log << "//////////////////Test wielu metod///////////////////" << endl;

	log << "//////////////////tworze proxy///////////////////" << endl;
	CalcProxy p(2);
	log << "//////////////////wolam future dodawania///////////////////" << endl;
	
	Future<int> f1 = p.AddInt(30,50);
	Future<int> f2 = p.SlowAddInt(34,51);
	Future<int> f3 = p.ReallyFrickinLongAddInt(3,58);
	Future<int> f4 = p.DivideInt(234,2);
	Future<double> f5 = p.DivideDouble(12312.23,5434.99);

	log << "//////////////////czekamy...///////////////////" << endl;
	
	log << "f1: " << f1.getValue() << endl;
	log << "f2: " << f2.getValue() << endl;
	log << "f3: " << f3.getValue() << endl;
	log << "f4: " << f4.getValue() << endl;
	log << "f5: " << f5.getValue() << endl;
}

void testGuard()
{
	Logger log("MAIN");
	log << "//////////////////Test guarda///////////////////" << endl;

	log << "//////////////////tworze proxy///////////////////" << endl;
	QueueProxy p(1);
	log << "//////////////////wolam future dodawania///////////////////" << endl;
	
	Future<void> f1 = p.Put("msg1");
	Future<void> f2 = p.Put("msg2");
	Future<void> f3 = p.Put("msg3");
	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
	log << "//////////////////Wyjmowanie z kolejki///////////////////" << endl;
	Future<std::string> f4 = p.Get();
	Future<std::string> f5 = p.Get();
	Future<std::string> f6 = p.Get();

	log << "//////////////////czekamy...///////////////////" << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
	log << "//////////////////getValue...///////////////////" << endl;
	log << "f4: " << f4.getValue() << endl;
	log << "f5: " << f5.getValue() << endl;
	log << "f6: " << f6.getValue() << endl;
}

void testVoidInvokes()
{
	Logger log("MAIN");
	log << "//////////////////Test metod void, zobaczymy, czy sie wykonaja///////////////////" << endl;

	log << "//////////////////tworze proxy///////////////////" << endl;
	QueueProxy p(1);
	log << "//////////////////wolam metody dodawania///////////////////" << endl;
	
	p.Put("msg1");
	p.Put("msg2");
	p.Put("msg3");
	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
	log << "//////////////////Wyjmowanie z kolejki///////////////////" << endl;
	Future<std::string> f4 = p.Get();
	Future<std::string> f5 = p.Get();
	Future<std::string> f6 = p.Get();

	log << "//////////////////czekamy...///////////////////" << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
	log << "//////////////////getValue...///////////////////" << endl;
	log << "f4: " << f4.getValue() << endl;
	log << "f5: " << f5.getValue() << endl;
	log << "f6: " << f6.getValue() << endl;
}

void testGuardMultipleThreads()
{
	Logger log("MAIN");
	log << "//////////////////Test guarda///////////////////" << endl;

	log << "//////////////////tworze proxy///////////////////" << endl;
	SyncQueueProxy p(3);
	log << "//////////////////wolam future dodawania///////////////////" << endl;
	
	Future<void> f1 = p.Put("msg1");
	Future<void> f2 = p.Put("msg2");
	Future<void> f3 = p.Put("msg3");
	Future<void> f4 = p.Put("msg4");
	Future<void> f5 = p.Put("msg5");
	Future<void> f6 = p.Put("msg6");
	Future<void> f7 = p.Put("msg7");
	Future<void> f8 = p.Put("msg8");
	Future<void> f9 = p.Put("msg9");
	log << "//////////////////czekamy...///////////////////" << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(10000));
	log << "//////////////////Wyjmowanie z kolejki///////////////////" << endl;
	for(int i=0;i<9;++i)
	{
		Future<std::string> f_ans = p.Get();
		log << "f_ans: " << f_ans.getValue() << endl;
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	}

	log << "//////////////////kasujemy wsio...///////////////////" << endl;
}

void testRefreshPeriod()
{
	Logger log("MAIN");
	log << "//////////////////Test refresh period///////////////////" << endl;

	log << "//////////////////tworze proxy///////////////////" << endl;
	SyncQueueProxy p(3,2000L);

	log << "//////////////////wolam future dodawania///////////////////" << endl;
	
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
	log << "//////////////////czekamy...///////////////////" << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(4000));
	log << "//////////////////Wyjmowanie z kolejki///////////////////" << endl;
	for(int i=0;i<10;++i)
	{
		Future<std::string> f_ans = p.Get();
		log << "f_ans: " << f_ans.getValue() << endl;
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	}

	log << "//////////////////kasujemy wsio...///////////////////" << endl;
}

//int main(int argc, char* argv[])
BOOST_AUTO_TEST_CASE(Main)
{	
	cout<<"TESTOWANIE ACTIVE OBJECT"<<endl;
	int prog= 1;
	while(prog) 
	{
		cout<<endl;
		cout<<"Wybierz test wpisujac odpowiednia liczbe: "<<endl<<endl;
		cout<<"\ttestSyncProxy\t\t-\t1"<<endl;
		cout<<"\ttestSimpleInvoke\t-\t2"<<endl;
		cout<<"\ttestException\t\t-\t3"<<endl;
		cout<<"\ttestCancel\t\t-\t4"<<endl;
		cout<<"\ttestSharedContent\t-\t5"<<endl;
		cout<<"\ttestSwapCallback\t-\t6"<<endl;
		cout<<"\ttestManyThreads\t\t-\t7"<<endl;
		cout<<"\ttestSingletonServant\t-\t8"<<endl;
		cout<<"\ttestPersistantFuture\t-\t9"<<endl;
		cout<<"\ttestManyMethods\t\t-\t10"<<endl;
		cout<<"\ttestGuard\t\t-\t11"<<endl;
		cout<<"\ttestVoidInvokes\t\t-\t12"<<endl;
		cout<<"\ttestGuardMultipleThreads-\t13"<<endl;
		cout<<"\ttestRefreshPeriod\t-\t14"<<endl<<endl;
		cout<<"\tWyjscie\t\t\t-\t0"<<endl;

		cin>>prog;
		cout<<endl;

		if(prog<0) prog=0;
		else if(prog==1) testSyncProxy();
		else if(prog==2) testSimpleInvoke();
		else if(prog==3) testException();
		else if(prog==4) testCancel();
		else if(prog==5) testSharedContent();
		else if(prog==6) testSwapCallback();
		else if(prog==7) testManyThreads();
		else if(prog==8) testSingletonServant();
		else if(prog==9) testPersistantFuture();
		else if(prog==10) testManyMethods();
		else if(prog==11) testGuard();
		else if(prog==12) testVoidInvokes();
		else if(prog==13) testGuardMultipleThreads();
		else if(prog==14) testRefreshPeriod();
	}
	system("PAUSE");
/*
	return EXIT_SUCCESS;*/
}

#endif
