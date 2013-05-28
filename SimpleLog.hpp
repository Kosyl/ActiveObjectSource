/** \file     SimpleLog.cpp
\brief    Funkcja do wygodnego wypisywania komunikatow
*/

#ifndef _SIMPLELOG_
#define _SIMPLELOG_


#ifdef _DEBUG 
#define DLOG(x) x
#else 
#define DLOG(x)
#endif

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>

//////////platform-dependent!
#include <windows.h>
#include <iostream>

using namespace boost::posix_time;

using namespace std;

class Logger
{

private:

	string modul_;
	bool printDetails_;
	stringstream ss;
	static auto_ptr<boost::mutex> mutex_;
	static auto_ptr<boost::condition_variable> next_;
	static auto_ptr<boost::thread::id> currentWriterId_;
	static bool allowNext_;
	short color_;

public:

	Logger(string s, short color=15):
		modul_(s),
		printDetails_(true),
		color_(color)
	{
		*currentWriterId_=boost::this_thread::get_id();
	}

	template<typename T>
	friend Logger& operator<<(Logger& o, T const& t);

	void emptyBuffer()
	{
		HANDLE  hConsole;
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo( hConsole, &csbi );
		SetConsoleTextAttribute(hConsole, color_);

		cout << ss.rdbuf() << std::flush;

		SetConsoleTextAttribute( hConsole, csbi.wAttributes );
		ss=stringstream();

		allowNext_=true;
		next_->notify_one();
	}

	void printPrefix()
	{
		printDetails_=false;

		const ptime now = microsec_clock::universal_time();
		const time_duration td = now.time_of_day();
		const long hours        = td.hours();
		const long minutes      = td.minutes();
		const long seconds      = td.seconds();
		const long milliseconds = (const long)(td.total_milliseconds() -	((hours * 3600 + minutes * 60 + seconds) * 1000));

		ss << hours+2 << ":" << setw(2) << minutes << ":" << setw(2) << seconds << "." << setw(3) << milliseconds << " | " << setw(5) << boost::this_thread::get_id() << " | "<< setw(10) << modul_ << ": ";
	}

	template<typename T>
	void write(T s) 
	{
		boost::mutex::scoped_lock lock(*mutex_);
		if(!allowNext_)
		{
			if(boost::this_thread::get_id()!=*currentWriterId_)
			{
				next_->wait(lock);
			}
		}
		*currentWriterId_=boost::this_thread::get_id();
		allowNext_=false;

		if(printDetails_)
		{
			printPrefix();
		}

		ss << s;
	}

	template<>
	void write(const char* s)
	{
		boost::mutex::scoped_lock lock(*mutex_);
		if(!allowNext_)
		{
			if(boost::this_thread::get_id()!=*currentWriterId_)
			{
				next_->wait(lock);
			}
		}
		*currentWriterId_=boost::this_thread::get_id();
		allowNext_=false;

		string str(s);
		lock.unlock();
		write(str);
	}

	template<>
	void write(string s)
	{
		boost::mutex::scoped_lock lock(*mutex_);
		if(!allowNext_)
		{
			if(boost::this_thread::get_id()!=*currentWriterId_)
			{
				next_->wait(lock);
			}
		}
		*currentWriterId_=boost::this_thread::get_id();
		allowNext_=false;

		if(s=="\n" || s.find("\n")!=string::npos)
		{
			printDetails_=true;
			ss << s;
			emptyBuffer();
			return;
		}
		else
		{
			if(printDetails_)
			{
				printPrefix();
			}
			ss << s;
		}
	}
};

auto_ptr<boost::mutex> Logger::mutex_(new boost::mutex());
auto_ptr<boost::condition_variable> Logger::next_(new boost::condition_variable());
bool Logger::allowNext_=true;
auto_ptr<boost::thread::id> Logger::currentWriterId_(new boost::thread::id);

template<typename T>
Logger& operator<<(Logger& o, T const& t)
{
	o.write(t);
	return o;
}

Logger& operator<<(Logger& o, std::ostream& (*f)(std::ostream&))
{
	o.write("\n");
	return o;
}


#endif