#include "SimpleLog.hpp"
#include "FutureContentCreator.hpp"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include "Proxy.hpp"
#include "Future.hpp"
#include "MethodRequest.hpp"

using namespace std;

using namespace ActiveObject;
//implementacja przykladowego servanta, ma progress!
class SimpleServant: public FutureContentCreator
{
private:
	boost::mutex mutex_;
public:

	SimpleServant()
	{}

	void print(string s)
	{
		boost::mutex::scoped_lock l(mutex_);
		cout<<s<<endl;
	}

	void Predict(string word)
	{
		srand((unsigned int)time(NULL));
		boost::this_thread::sleep(boost::posix_time::milliseconds(3000));
		{
			boost::mutex::scoped_lock l(mutex_);
			cout<<"\""<<word<<",taaaaaak..."<<endl;
		}
		boost::this_thread::sleep(boost::posix_time::milliseconds(2000));
		{
			boost::mutex::scoped_lock l(mutex_);
			cout<<"'Hmmm, "<<word<<", "<<word<<", "<<word<<", ile lat jeszcze pozyjesz?"<<endl;
		}
		boost::this_thread::sleep(boost::posix_time::milliseconds(2500));
		{
			boost::mutex::scoped_lock l(mutex_);
			cout<<"'Taaaak, to bardzo ciekawe... "<<word<<", zaraz poznasz swoja przyszlosc!"<<endl;
		}
		boost::this_thread::sleep(boost::posix_time::milliseconds(3000));
		int v = rand() % 20 +5;
		{
			boost::mutex::scoped_lock l(mutex_);
			cout<<"Maciej wrozbita mowi, ze "<<word<< " umrze za " <<v<<" lat."<<endl;
		}

	}

};


class SimpleProxy: public Proxy<SimpleServant,ServantFactoryCreator>
{

public:

	SimpleProxy(int numThreads=1):
		Proxy(numThreads)
	{}

	Future<void> print(string word) 
	{
		return enqueue<void> (boost::bind(&SimpleServant::print,_1,word));
	}
	Future<void> predict(string word) 
	{
		return enqueue<void> (boost::bind(&SimpleServant::Predict,_1,word));
	}
};


int main(int argc, char* argv[])
{	
	int tmp=1;
	cout<<"Wpisz imie osoby, ktorej przyszlosc chcesz poznac. Wrozbita Maciej powie ci najwieksze sekrety wiecznosci!"<<endl;
	cout<<"Wrozbita Maciej moze przewidywac przyszlosc dla 3 osob jednoczesnie!"<<endl;
	cout<<"Wpisz 'Maciej', zeby sploszyc wrozbite!"<<endl;
	string word;
	SimpleProxy* proxy= new SimpleProxy(3);
	do
	{
		cin.clear();
		cin>>word;
		cin.clear();
		if(word=="Maciej")break;
		proxy->predict(word);
		cout<<"Chcesz pozac przyszlosc osoby o imieniu '"<<word<<"'. Wrozbita zastanawia sie."<<endl;
		cout<<"Uwazaj, jesli wpiszesz 'Maciej', to wrozbita sie obrazi i wyjdzie!.\nWpisz nastepne imie:"<<endl;
	}
	while (1);
	cout << "Wrozbita Maciej uprzejmie dokonczy przepowiadanie..." << endl;
	delete proxy;
	cout << "Koniec przepowiedni na dzis!" << endl;
	return EXIT_SUCCESS;
}

