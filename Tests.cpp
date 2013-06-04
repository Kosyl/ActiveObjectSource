#ifndef _TESTS_
#define _TESTS_

#include "Future.hpp"
#include "Example1.hpp"
#include "SimpleLog.hpp"
#include "Example2_kolejka.hpp"
#include <string>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/test/minimal.hpp>
#include <boost/exception_ptr.hpp>

#define CHECK(exp) {bool tmp=exp;log.lock();(tmp)? static_cast<void>(0): boost::minimal_test::report_error(#exp,__FILE__,__LINE__, BOOST_CURRENT_FUNCTION);log.unlock();}
#define REQUIRE(exp){ bool tmp=exp;log.lock();(tmp)? static_cast<void>(0): boost::minimal_test::report_critical_error(#exp,__FILE__,__LINE__,BOOST_CURRENT_FUNCTION);log.unlock();}

using namespace std;
using namespace ActiveObject;
using namespace boost::unit_test;
using namespace boost::unit_test_framework;

void testFuture()
{
	Logger log("T:Future");
	log << "Test future" << endl;
	log << "Tworzymy prosty content i Future wskazujacy na niego, przeprowadzamy rozne operacje" << endl;
	boost::shared_ptr<FutureContent> fc(new FutureContent());
	Future<int> f(fc);
	log << "testy wstepne..." << endl;
	REQUIRE(f.getException()==NULL);
	REQUIRE(f.hasException()==false);
	REQUIRE(f.isDone()==false);
	CHECK(f.getProgress()==0.0);
	CHECK(f.getState()==QUEUED);

	log << "setState..." << endl;
	fc->setState(INPROGRESS);
	CHECK(f.getState()==INPROGRESS);

	log << "setProgress..." << endl;
	fc->setProgress(0.2);
	CHECK(f.getProgress()==0.2);
	REQUIRE(f.isDone()==false);
	REQUIRE(f.hasException()==false);

	log << "progress listener..." << endl;
	struct listener
	{
		listener(boost::shared_ptr<double> p):
			p_(p){}
		void operator()(double p)
		{
			*p_=p;
		}
		boost::shared_ptr<double> p_;
	};
	boost::shared_ptr<double> progr(new double);
	f.setFunction(listener(progr));
	fc->setProgress(0.45);
	CHECK(f.getProgress()==0.45);
	CHECK(*progr==0.45);

	log << "setValue..." << endl;
	fc->setValue(42);
	REQUIRE(f.isDone()==true);
	REQUIRE(f.getValue()==42);
	REQUIRE(f.hasException()==false);
	CHECK(f.getState()==DONE);
	log << "exception..." << endl;

	fc->setException(boost::copy_exception(NonPositivePeriodException()));

	REQUIRE(f.hasException()==true);
	CHECK(f.getState()==EXCEPTION);
	REQUIRE(f.getException()!=NULL);

	try
	{
		f.getValue();
	}
	catch (NonPositivePeriodException& e)
	{
		REQUIRE(e.what()=="Refresh period has to be greater than 0!");
	}
	catch(std::exception const& e)
	{
		log << "Wyjatek:" << e.what() << endl;
		BOOST_FAIL("Zlapano zly wyjatek!");
	}
	log << "sprzatam..." << endl;
}

void testSyncProxy()
{
	Logger log("T:ProxySync");
	log << "Test proxy" << endl;
	log << "3x tworzone jest Proxy o 3 watkach. Sprawdz ilosc konstruktorow i destruktorow" << endl;
	for(int i=0;i<3;++i)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));
		log << "tworze proxy..." << endl;
		CalcProxy p(3);
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));
		log << "kasuje proxy..." << endl;
	}
	log << "sprzatam..." << endl;
}

void testSimpleInvoke()
{
	Logger log("T:Invoke");
	log << "Test prostego wywolania" << endl;

	log << "tworze proxy..." << endl;
	CalcProxy p(1);
	log << "wolam future dodawania..." << endl;
	struct call
	{
		void operator ()(double x)
		{
			cout << "Progress listener: " << x << endl;
		}
	} callback;

	Future<int> f = p.AddInt(3,5);
	f.setFunction(callback);

	log << "czekam......" << endl;
	REQUIRE(f.isDone()==false);
	REQUIRE(f.hasException()==false);
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	try
	{
		REQUIRE(f.getValue()==8);
	}
	catch(...)
	{
		BOOST_FAIL("Nastapil wyjatek podczas wywolania");
	}

	REQUIRE(f.hasException()==false);
	REQUIRE(f.isDone()==true);
	REQUIRE(f.getProgress()==1.0);
	log << "wynik: " << f.getValue() << endl;
	
	int x = 4+f;
	log << "dodanie 4+f (test rzutowania):" << x << endl;
	REQUIRE(4+f==12);
	log << "sprzatam..." << endl;
}


void testException()
{
	Logger log("T:Exception");
	log << "Test exception..." << endl;

	log << "tworze proxy..." << endl;
	CalcProxy p(1);

	log << "wolam future dzielenia przez 0..." << endl;

	Future<int> f2 = p.DivideInt(3,0);
	log << "czekam..." << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	REQUIRE(f2.hasException()==true);
	REQUIRE(f2.getState()==EXCEPTION);
	log << "wynik..." << endl;
	try
	{
		f2.getValue();
	}
	catch(exception& e)
	{
		log << "Wyjatek: " << e.what() << endl;
		try
		{
			throw std::overflow_error("Divide by zero exception");
		}
		catch(std::overflow_error x)
		{
			REQUIRE(string(e.what())==string(x.what()));
		}
	}
	log << "sprzatam..." << endl;
}

void testCancel()
{
	Logger log("T:Cancel");
	log << "Test cancel" << endl;

	log << "tworze proxy..." << endl;
	CalcProxy p(1);
	log << "wolam future dlugiego dodawania..." << endl;

	Future<int> f = p.ReallyLongAddInt(3,5);
	log << "czekam..." << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
	log << "kasuje..." << endl;
	f.cancelRequest();
	REQUIRE(f.hasException()==true);
	REQUIRE(f.getState()==CANCELLED);
	log << "wynik..." << endl;
	try
	{
		f.getValue();
	}
	catch(RequestCancelledException& e)
	{
		log << "Wyjatek: " << e.what() << endl;
		try
		{
			throw RequestCancelledException();
		}
		catch(RequestCancelledException& x)
		{
			REQUIRE(string(e.what())==string(x.what()));
		}
	}
	catch(...)
	{
		BOOST_FAIL("Zlapano zly wyjatek");
	}
	log << "sprzatam..." << endl;
}

void testSharedContent()
{
	Logger log("T:Sh.Cont.");
	log << "Test wspoldzielonego contentu" << endl;

	log << "tworze proxy..." << endl;
	CalcProxy p(1);
	log << "wolam future dlugiego dodawania..." << endl;

	Future<int> f = p.ReallyLongAddInt(3,5);
	log << "czekam chwile..." << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(3000));
	log << "tworzymy nowe future w oparciu o obecne..." << endl;
	Future<int>f2(f);
	log << "sprawdzamy, czy sa analogiczne..." << endl;
	log << "wyniki moga sie roznic, jesli writer wcisnie sie miedzy wywolania getow na future, dlatego tylko checki..." << endl;
	CHECK(f.getException()==f2.getException());
	CHECK(f.getProgress()==f2.getProgress());
	CHECK(f.getState()==f2.getState());
	CHECK(f.isDone()==f2.isDone());
	CHECK(f2.isDone()==false);
	CHECK(f2.isDone()==false);
	CHECK(f.hasException()==f2.hasException());
	CHECK(f2.hasException()==false);
	log << "kasuje jeden future..." << endl;
	f.cancelRequest();
	REQUIRE(f.hasException()==true);
	REQUIRE(f.getState()==CANCELLED);
	REQUIRE(f2.hasException()==false);
	REQUIRE(f2.getState()==INPROGRESS);
	CHECK(f2.isDone()==false);
	log << "czekam na wykonanie..." << endl;
	try
	{
		f2.getValue();
	}
	catch(exception&)
	{
		BOOST_FAIL("Drugie wywolanie zakonczylo sie wyjatkiem!");
	}
	log << "sprawdzam future po wykonaniu..." << endl;
	REQUIRE(f2.getValue()==8);
	REQUIRE(f2.getState()==DONE);
	REQUIRE(f2.getProgress()==1.0);
	log << "sprzatam..." << endl;
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
	Future<int> f = p.ReallyLongAddInt(30,50);
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

	Future<int> f1 = p.ReallyLongAddInt(30,50);
	Future<int> f2 = p.ReallyLongAddInt(34,51);
	Future<int> f3 = p.ReallyLongAddInt(3,58);
	Future<int> f4 = p.ReallyLongAddInt(1,234);
	Future<int> f5 = p.ReallyLongAddInt(23,523);
	Future<int> f6 = p.ReallyLongAddInt(43,34);
	Future<int> f7 = p.ReallyLongAddInt(100,34);
	Future<int> f8 = p.ReallyLongAddInt(2,34);

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

	Future<int> f = p->ReallyLongAddInt(3,5);

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
	Future<int> f3 = p.ReallyLongAddInt(3,58);
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

int test_main(int argc, char* argv[])
{	
	cout<<"TESTOWANIE ACTIVE OBJECT"<<endl;
	testFuture();
	testSyncProxy();
	testSimpleInvoke();
	testException();
	testCancel();
	testSharedContent();
	/*else if(prog==6) testSwapCallback();
	else if(prog==7) testManyThreads();
	else if(prog==8) testSingletonServant();
	else if(prog==9) testPersistantFuture();
	else if(prog==10) testManyMethods();
	else if(prog==11) testGuard();
	else if(prog==12) testVoidInvokes();
	else if(prog==13) testGuardMultipleThreads();
	else if(prog==14) testRefreshPeriod();*/

	return EXIT_SUCCESS;
}

#endif
