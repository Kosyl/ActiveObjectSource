#ifndef _SIMPLELOG_
#define _SIMPLELOG_

/**
* In DEBUG mode it orders all the classes to log progress of their methods.
*/
#ifdef _DEBUG 
#define DLOG(x) x
#else 
#define DLOG(x)
#endif

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>

/**
* Platform-dependent import allowing colorful command output
*/
#ifdef _WINPLATFORM
#include <windows.h>
#endif

#include <iostream>

using namespace boost::posix_time;

namespace ActiveObject
{

	using namespace std;

	/**
	* @brief Thread-safe convenient logger to print statements.
	*/
	class Logger
	{

	private:

		/**
		* @brief Class prefix (e.g. "Scheduler")
		*/
		string modul_;

		/**
		* @brief true, if the console should print the prefix
		*/
		bool printDetails_;

		/**
		* @brief Message buffer
		*/
		stringstream ss;

		/**
		* @brief Synchronization variable
		*/
		static unique_ptr<boost::mutex> mutex_;

		/**
		* @brief Notifies waiting threads
		*/
		static unique_ptr<boost::condition_variable> next_;

		/**
		* @brief ThreadID of current writer. It is re-set after the end of the line.
		*/
		static unique_ptr<boost::thread::id> currentWriterId_;

		/**
		* @brief Used to stop the logging externally
		*/
		static unique_ptr<boost::mutex::scoped_lock> stopLogging_;

		/**
		* @brief Allows the next thread in
		*/
		static bool allowNext_;

		/**
		* @brief Color of the output for current class (or instance in general)
		*/
		short color_;

	public:

		/**
		* @brief Default constructor
		*/
		Logger():
			modul_(""),
			printDetails_(true),
			color_(0)
		{
			*currentWriterId_=boost::this_thread::get_id();
		}

		/**
		* @brief Constructor
		* @param s module name
		* @param color output color number (platform-dependent)
		*/
		Logger(string s, short color=15):
			modul_(s),
			printDetails_(true),
			color_(color)
		{
			*currentWriterId_=boost::this_thread::get_id();
		}

		/**
		* @brief friend output operator
		*/
		template<typename T>
		friend Logger& operator<<(Logger& o, T const& t);

		/**
		* @brief external order to stop logging
		*/
		void lock()
		{
			if(!stopLogging_)stopLogging_= unique_ptr<boost::mutex::scoped_lock>(new boost::mutex::scoped_lock(*mutex_));
		}

		/**
		* @brief sets the color of the Logger instance
		* @param i color
		*/
		void setColor(int i)
		{
			color_=i;
		}

		/**
		* @brief sets the prefix module name
		* @param s module name
		*/
		void setName(string s)
		{
			modul_=s;
		}

		/**
		* @brief external order to turn logging back on
		*/
		void unlock()
		{
			if(stopLogging_)stopLogging_.reset();
		}

		/**
		* @brief prints the current buffer to cout
		* Coloring works on Windows and Linux, alternatively prints in the default color
		*/
		void emptyBuffer()
		{
#if defined (_WINPLATFORM) && !defined (_LINPLATFORM)
			HANDLE  hConsole;
			hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			CONSOLE_SCREEN_BUFFER_INFO csbi;
			GetConsoleScreenBufferInfo( hConsole, &csbi );
			SetConsoleTextAttribute(hConsole, color_);

			cout << ss.rdbuf() << std::flush;

			SetConsoleTextAttribute( hConsole, csbi.wAttributes );
#endif
#if !defined (_WINPLATFORM) && defined (_LINPLATFORM)
			std::string color = "\033[0;"+std::to_string(30+color_%8)+"m";
			std::cout << color << ss.rdbuf() << std::flush;
			std::cout << ("\033[0;m");
#endif
#if !defined (_WINPLATFORM) && !defined (_LINPLATFORM)
			cout << ss.rdbuf() << std::flush;
#endif
			ss.clear();

			allowNext_=true;
			next_->notify_one();
		}

		/**
		* @brief prints time, ThreadID and name of the writing module (e.g. class)
		*/
		void printPrefix()
		{
			printDetails_=false;

			const ptime now = microsec_clock::universal_time();
			const time_duration td = now.time_of_day();
			const long hours        = td.hours();
			const long minutes      = td.minutes();
			const long seconds      = td.seconds();
			const long milliseconds = (const long)(td.total_milliseconds() -	((hours * 3600 + minutes * 60 + seconds) * 1000));

			ss << hours+2 << ":" << setw(2) << minutes << ":" << setw(2) << seconds << "." << setw(3) << milliseconds << " | " << setw(5) << boost::this_thread::get_id() << " | "<< setw(13) << modul_ << ": ";
		}

		/**
		* @brief helper struct which allows to choose the correct private write function
		*/
		template<typename T>
		struct identity { typedef T type; };

		/**
		* @brief general write function
		* @param s object to be written to cout (e.g. string, char*, std::endl, int...)
		*/
		template<typename T>
		void write(T s) 
		{
			write(s,identity<T>());
		}

	private:

		/**
		* @brief write function for non-string objects
		* @param s object to be written
		* @param i type of the object, which chooses the correct function
		*/
		template<typename T>
		void write(T s, identity<T> i) 
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

		/**
		* @brief write function for char*, redirects it to write(string)
		* @param s char* to be written
		* @param i type of the object (char*) which chooses the correct function
		*/
		void write(const char* s, identity<const char*> i)
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

		/**
		* @brief write function for string
		* @param s std::string to be written
		* @param i type of the object (std::string) which chooses the correct function
		*/
		void write(string s, identity<std::string> i)
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
	};//Logger

	unique_ptr<boost::mutex> Logger::mutex_(new boost::mutex());
	unique_ptr<boost::condition_variable> Logger::next_(new boost::condition_variable());
	unique_ptr<boost::thread::id> Logger::currentWriterId_(new boost::thread::id);
	unique_ptr<boost::mutex::scoped_lock> Logger::stopLogging_=0;
	bool Logger::allowNext_=true;

	/**
	* @brief Logger's friend << operator to use the logger
	* @param o logger instance
	* @param t object to be written to cout
	*/
	template<typename T>
	Logger& operator<<(Logger& o, T const& t)
	{
		o.write(t);
		return o;
	}

	/**
	* @brief Logger's friend << operator specialization for std::endl
	* @param o logger instance
	* @param t object to be written to cout
	*/
	Logger& operator<<(Logger& o, std::ostream& (*f)(std::ostream&))
	{
		o.write("\n");
		return o;
	}

}//ActiveObject
#endif
