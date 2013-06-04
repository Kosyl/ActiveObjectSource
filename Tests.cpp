#ifndef _TESTS_
#define _TESTS_

#include "Future.hpp"
#include "Example1.hpp"
#include "SimpleLog.hpp"
#include "Example2_kolejka.hpp"
#include <string>
#include <set>
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

template <class T>
void pushMR (ActivationQueue<T>* q) 
{
	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
	boost::shared_ptr<FutureContent> content(new FutureContent());
	MethodRequest<double,CalcServant>* mr = new MethodRequest<double, CalcServant>(boost::bind(&CalcServant::DivideDouble,_1,12,3), content);
	q->push(mr);  
}

void testAQ() 
{
	Logger log("T: AQ:");
	log << "Test kolejki bez metody guard..." << endl;
	ActivationQueue<CalcServant>* q= new ActivationQueue<CalcServant>();
	REQUIRE(q->empty());
	log << "Tworzymy Servanta. Potem MethodRequest i wkladamy do kolejki..." << endl;
	ServantFactoryCreator<CalcServant> fact;
	boost::shared_ptr<FutureContent> content(new FutureContent());
	MethodRequest<int,CalcServant>* mr = new MethodRequest<int, CalcServant>(boost::bind(&CalcServant::AddInt,_1,2,3), content);
	q->push(mr);
	REQUIRE(!q->empty());
	REQUIRE(q->size()==1);
	boost::shared_ptr<FutureContent> content2(new FutureContent());
	MethodRequest<double,CalcServant>* mr2 = new MethodRequest<double, CalcServant>(boost::bind(&CalcServant::DivideDouble,_1,9,3), content2);
	q->push(mr2);
	REQUIRE(!q->empty());
	REQUIRE(q->size()==2);
	log << "Proba zdjecia z kolejki..." <<endl;
	q->pop(fact.getServant());
	REQUIRE(q->size()==1);
	q->pop(fact.getServant());
	log <<"Proba zdjecia z pustej kolejki..." << endl;
	boost::thread thread_=boost::thread(boost::bind(pushMR<CalcServant>,q));
	q->pop(fact.getServant());
	log << "Koniec kolejki..." << endl;
	q->End();
	log << "sprzatamy..." << endl;
}

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
		REQUIRE(string(e.what())=="Refresh period has to be greater than 0!");
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
	Logger log("T:Spw Call");
	log << "Test podmiany callbacka" << endl;
	log << "Test \"do popatrzenia\", jak mozna w locie podmienic funkcje reagujaca na progress" << endl;
	log << "tworze proxy..." << endl;
	CalcProxy p(1);
	log << "wolam future dodawania, ustawiam listenera..." << endl;
	struct call
	{
		void operator ()(double x)
		{
			cout << "Progress listener: " << x << endl;
		}
	} callback;
	Future<int> f = p.ReallyLongAddInt(30,50);
	f.setFunction(callback);

	log << "czekam..." << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(2500));

	log << "zmieniam callback..." << endl;
	struct call2
	{
		void operator ()(double x)
		{
			cout << "Nowy Wspanialy Lepszy Progress listener: " << x << endl;
		}
	} callback2;
	f.setFunction(callback2);

	log << "czekam..." << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(2500));

	log << "zmieniam callback..." << endl;
	struct call3
	{
		void operator ()(double x)
		{
			cout << "KRZYCZACY NOWY WSPANIALY LISTENER PROGRESSU: " << x << endl;
		}
	} callback3;
	f.setFunction(callback3);

	log << "blokuje i czekam do konca..." << endl;
	f.getValue();
	log << "sprzatam..." << endl;
}

void testManyThreads()
{
	Logger log("T: MT");
	log << "Test: wywolywanie wielu metod na wielu watkach" << endl;

	log << "tworze proxy..." << endl;
	CalcProxy p(3);
	log << "wolam 8x future dodawania..." << endl;

	Future<int> f1 = p.ReallyLongAddInt(30,50);
	Future<int> f2 = p.ReallyLongAddInt(34,51);
	Future<int> f3 = p.ReallyLongAddInt(3,58);
	Future<int> f4 = p.ReallyLongAddInt(1,234);
	Future<int> f5 = p.ReallyLongAddInt(23,523);
	Future<int> f6 = p.ReallyLongAddInt(43,34);
	Future<int> f7 = p.ReallyLongAddInt(100,34);
	Future<int> f8 = p.ReallyLongAddInt(2,34);

	log << "czekamy..." << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(2000));
	log << "pierwsze 3 future powinny byc INPROGRESS..." << endl;
	REQUIRE(f1.getState()==INPROGRESS);
	REQUIRE(f2.getState()==INPROGRESS);
	REQUIRE(f3.getState()==INPROGRESS);
	log << "czekamy na wyniki pierwsyzch trzech Future..." << endl;
	log << "f1: " << f1.getValue() << endl;
	log << "f2: " << f2.getValue() << endl;
	log << "f3: " << f3.getValue() << endl;
	REQUIRE(f1.getValue()==80);
	REQUIRE(f2.getValue()==85);
	REQUIRE(f3.getValue()==61);
	log << "czekamy..." << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(2000));
	log << "kolejne 3 future powinny byc INPROGRESS..." << endl;
	REQUIRE(f4.getState()==INPROGRESS);
	REQUIRE(f5.getState()==INPROGRESS);
	REQUIRE(f6.getState()==INPROGRESS);
	log << "czekamy na wyniki kolejnych trzech Future..." << endl;
	log << "f4: " << f4.getValue() << endl;
	log << "f5: " << f5.getValue() << endl;
	log << "f6: " << f6.getValue() << endl;
	REQUIRE(f4.getValue()==235);
	REQUIRE(f5.getValue()==546);
	REQUIRE(f6.getValue()==77);
	log << "czekamy..." << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(2000));
	log << "ostanie 2 future powinny byc INPROGRESS..." << endl;
	REQUIRE(f7.getState()==INPROGRESS);
	REQUIRE(f8.getState()==INPROGRESS);
	log << "czekamy na wyniki ostatnich Future..." << endl;
	log << "f7: " << f7.getValue() << endl;
	log << "f8: " << f8.getValue() << endl;
	REQUIRE(f7.getValue()==134);
	REQUIRE(f8.getValue()==36);
	log << "sprzatam..." << endl;
}

void testSingletonServant()
{
	Logger log("T:SinglServ");
	log << "Test servanta-\"singletona\"" << endl;
	log << "Proxy ma 3 watki obslugujace 1 synchronizowanego servanta" << endl;
	log << "Tylko 1 powinien sie wykonywac naraz" << endl;
	log << "tworze proxy..." << endl;
	SyncCalcProxy p(3);
	log << "wolam 4x future dodawania..." << endl;

	Future<int> f1 = p.SlowAddInt(30,50);
	Future<int> f2 = p.SlowAddInt(34,51);
	Future<int> f3 = p.SlowAddInt(3,58);
	Future<int> f4 = p.SlowAddInt(1,234);

	log << "czekamy..." << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	log << "pierwsze 3 future powinny byc INPROGRESS..." << endl;
	log << "ich blokowanie zachodzi w Servancie, ale de facto sie wykonuja..." << endl;
	REQUIRE(f1.getState()==INPROGRESS);
	REQUIRE(f2.getState()==INPROGRESS);
	REQUIRE(f3.getState()==INPROGRESS);
	log << "czekamy na 1sze Future..." << endl;
	log << "f1: " << f1.getValue() << endl;
	log << "po 1szym Future powinno odpalic sie ostanie (i od razu zablokowac na muteksie Servanta)..." << endl;
	log << "damy mu chwile, zeby sie zaladowac..." << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	REQUIRE(f4.getState()==INPROGRESS);
	REQUIRE(f2.getState()==INPROGRESS);
	REQUIRE(f3.getState()==INPROGRESS);
	log << "f2: " << f2.getValue() << endl;
	log << "f3: " << f3.getValue() << endl;
	log << "f4: " << f4.getValue() << endl;
	log << "sprawdzamy czy wszysko ok..." << endl;
	REQUIRE(f1.getState()==DONE);
	REQUIRE(f2.getState()==DONE);
	REQUIRE(f3.getState()==DONE);
	REQUIRE(f4.getState()==DONE);
	log << "sprzatamy..." << endl;
}

void testPersistantFuture()
{
	Logger log("T: PersFut");
	log << "Test dlugotrwalego future" << endl;
	log << "Upewnimy sie, ze Future istnieje tez po zamknieciu Proxy" << endl;
	log << "tworze proxy..." << endl;
	CalcProxy* p = new CalcProxy(1);
	log << "wolam future dlugiego dodawania..." << endl;

	Future<int> f = p->ReallyLongAddInt(3,5);

	log << "czekam na wartosc w trybie blokujacym..." << endl;
	try
	{
		log << f.getValue() << endl;
		REQUIRE(f.getValue()==8);
	}
	catch(exception&)
	{
		BOOST_FAIL("Wyjatek podczas wywolania");
	}

	boost::this_thread::sleep(boost::posix_time::milliseconds(2000));

	log << "kasuje proxy..." << endl;
	delete p;

	boost::this_thread::sleep(boost::posix_time::milliseconds(2000));

	log << "Ponowna proba odczytania wartosci..." << endl;
	try
	{
		log << f.getValue() << endl;
		REQUIRE(f.getValue()==8);
	}
	catch(exception&)
	{
		BOOST_FAIL("Po skasowaniu proxy wystapil wyjatek podczas wywolania getValue");
	}

	log << "cancel powinien skasowac content, patrzec na destruktory..." << endl;
	f.cancelRequest();
	log << "proba odczytania skasowanej wartosci rzuca wyjatek RequestCancelled..." << endl;
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
		BOOST_FAIL("Zlapano zly wyjatek przy probie odczytania warotsci skasowanej");
	}
	log << "sprzatam..." << endl;
}

void testManyMethods()
{
	Logger log("T: MultMeth");
	log << "Test wielu roznych metod na dwoch watkach..." << endl;

	log << "tworze proxy..." << endl;
	CalcProxy p(2);
	log << "wolam future dzialan..." << endl;

	Future<int> f1 = p.AddInt(30,50);
	Future<int> f2 = p.SlowAddInt(34,51);
	Future<int> f3 = p.ReallyLongAddInt(3,58);
	Future<int> f4 = p.DivideInt(234,2);
	Future<int> f0 = p.DivideInt(234,0);
	Future<double> f5 = p.DivideDouble(12312.23,5434.99);

	log << "czekamy..." << endl;

	log << "f1: " << f1.getValue() << endl;
	log << "f2: " << f2.getValue() << endl;
	log << "f3: " << f3.getValue() << endl;
	log << "f4: " << f4.getValue() << endl;
	log << "f5: " << f5.getValue() << endl;
	REQUIRE(f1.getState()==DONE);
	REQUIRE(f2.getState()==DONE);
	REQUIRE(f3.getState()==DONE);
	REQUIRE(f4.getState()==DONE);
	REQUIRE(f5.getState()==DONE);
	REQUIRE(f0.getState()==EXCEPTION);
	try
	{
		f0.getValue();
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

void testGuard()
{
	Logger log("T: Guard");
	log << "Test guarda..." << endl;
	log << "Servant to szufladka na 2 komunikaty string" << endl;
	log << "Put sprawdza czy jest pelna, jesli tak, to nie odpala sie" << endl;
	log << "Get sprawdza, czy jest pusta i jw." << endl;
	log << "tworze proxy..." << endl;
	QueueProxy p(1);
	log << "wolam 3 puty" << endl;

	Future<void> f1 = p.Put("msg1");
	Future<void> f2 = p.Put("msg2");
	Future<void> f3 = p.Put("msg3");
	log << "czekamy" << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(3000));
	log << "2 Puty powinny sie wykonac, trzeci powinien zablokowac sie na Guard..." << endl;
	REQUIRE(f1.getState()==DONE);
	REQUIRE(f2.getState()==DONE);
	REQUIRE(f3.getState()==QUEUED);

	log << "Wyjmowanie z kolejki..." << endl;
	log << "Jedno Get powinno sie wykonac, a nastepnie pozwolic na wykonanie Put..." << endl;

	Future<std::string> f4 = p.Get();
	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
	REQUIRE(f3.getState()==DONE);
	REQUIRE(f4.getState()==DONE);
	REQUIRE(f4.getValue()=="msg1");

	log << "Wyjmowanie z kolejki pozostalych wiadomosci..." << endl;
	Future<std::string> f5 = p.Get();
	Future<std::string> f6 = p.Get();

	log << "czekamy..." << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
	log << "sprawdzamy wartosci..." << endl;

	REQUIRE(f5.getState()==DONE);
	REQUIRE(f5.getValue()=="msg2");
	REQUIRE(f6.getState()==DONE);
	REQUIRE(f6.getValue()=="msg3");
}

void testVoidInvokes()
{
	Logger log("T: Fut<void>");
	log << "Test metod void..." << endl;

	log << "tworze proxy..." << endl;
	QueueProxy p(1);
	log << "wolam metody dodania komunikatow do kolejki..." << endl;
	log << "dwa sie wykonaja, trzeci poczeka na guarda w kolejce..." << endl;
	log << "po wywolaniach powinny wolac sie destruktory contentow!..." << endl;
	p.Put("msg1");
	p.Put("msg2");
	p.Put("msg3");
	log << "czekamy" << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
	log << "Wyjmowanie z kolejki..." << endl;
	Future<std::string> f4 = p.Get();
	Future<std::string> f5 = p.Get();
	Future<std::string> f6 = p.Get();

	log << "czekamy..." << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
	log << "getValues..." << endl;
	log << "f4: " << f4.getValue() << endl;
	log << "f5: " << f5.getValue() << endl;
	log << "f6: " << f6.getValue() << endl;
	REQUIRE(f4.getState()==DONE);
	REQUIRE(f4.getValue()=="msg1");
	REQUIRE(f5.getState()==DONE);
	REQUIRE(f5.getValue()=="msg2");
	REQUIRE(f6.getState()==DONE);
	REQUIRE(f6.getValue()=="msg3");
}

int test_main(int argc, char* argv[])
{	
	cout<<"TESTOWANIE ACTIVE OBJECT"<<endl;
	testAQ();
	testFuture();
	testSyncProxy();
	testSimpleInvoke();
	testException();
	testCancel();
	testSharedContent();
	testSwapCallback();
	testManyThreads();
	testSingletonServant();
	testPersistantFuture();
	testManyMethods();
	testGuard();
	testVoidInvokes();

	return EXIT_SUCCESS;
}

#endif
