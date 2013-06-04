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
	bool guard_;
	int counter;
public:

	SimpleServant():
	  guard_(true),
	  counter(0)
	{}

	SimpleServant(const SimpleServant& rhs)
	{}

	void print(string word)
	{
		srand(time(NULL));
		boost::this_thread::sleep(boost::posix_time::milliseconds(3000));
		cerr<<"Wrozbita Maciej jest w 50% procesu zastanawiania sie 'Hmmm, "<<word<<", ile lat jeszcze pozyjesz?"<<endl;
		boost::this_thread::sleep(boost::posix_time::milliseconds(3000));
		int v = rand() % 20 +5;
		cerr<<"Maciej wrozbita mowi, ze "<<word<< " umrze za " <<v<<" lat"<<endl;
		
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

};


int main(int argc, char* argv[])
{	
	
	int tmp=1;
	cout<<"Wpisz imie"<<endl;
	string word;
	SimpleProxy* proxy= new SimpleProxy(2);
	while (tmp) 
	{
		cin>>word;
		proxy->print(word);
		cout<<"Wpisz nastepne imie"<<endl;
		cout<<"Uwazaj, jesli wpiszesz 'Maciej', to wrozbita sie obrazi i wyjdzie."<<endl;
		cin>>word;
		if(word=="Maciej") tmp=0;
		else proxy->print(word);
	}
	return EXIT_SUCCESS;
}

