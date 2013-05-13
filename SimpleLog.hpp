/** \file     SimpleLog.cpp
\brief    Funkcja do wygodnego wypisywania komunikatow
*/

#ifndef _SIMPLELOG_
#define _SIMPLELOG_

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;

using namespace std;

class Logger
{
private:
	string modul_;
	bool printDetails_;
public:
	Logger(string s):
		modul_(s),
		printDetails_(true)
	{}

	template<typename T>
	friend Logger& operator<<(Logger& o, T const& t);

	template<typename T>
	void write(T s)
	{
		cout << s;
	}

	void write(string s)
	{
		if(s=="\n" || s.find("\n")!=string::npos)
		{
			printDetails_=true;
			cout << s;
			return;
		}
		else
		{
			if(printDetails_)
			{
				printDetails_=false;

				const ptime now = microsec_clock::universal_time();
				const time_duration td = now.time_of_day();
				const long hours        = td.hours();
				const long minutes      = td.minutes();
				const long seconds      = td.seconds();
				const long milliseconds = td.total_milliseconds() -	((hours * 3600 + minutes * 60 + seconds) * 1000);

				cout << hours << ":" << minutes << ":" << seconds << "." << milliseconds << " " << modul_ << "\t: ";
			}
			cout << s;
		}

	}
};

template<typename T>
Logger& operator<<(Logger& o, T const& t)
{
	o.write(t);
	return o;
}

Logger& operator<<(Logger& o, std::ostream& (*f)(std::ostream&))
{
	o.write(f);
	return o;
}



#endif