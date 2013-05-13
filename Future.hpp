#ifndef _FUTURE_
#define _FUTURE_

#include <boost\smart_ptr\shared_ptr.hpp>
#include <boost\thread.hpp>

using namespace std;

template<typename T>
class Promise;

template<typename T>
class Future
{
	friend class Promise<T>;
	typedef boost::shared_ptr<T> PT;
	typedef boost::mutex::scoped_lock sLock;

private:
	PT pValue_;
	volatile bool isDone_;
	boost::shared_ptr<exception> exception_;
	double progress_;
	bool haltOrder_;

	boost::mutex mutex;
	boost::condition_variable cv;

	void setProgress(const double& progress)
	{
		sLock lock(mutex)
		cout << "FUTURE: set progress: " << progress << endl;
		progress_ = progress;
	}

	void setValue(PT& val)
	{
		sLock lock(mutex);
		cout << "FUTURE: set value: " << *val << endl;
		pValue_(val);
		isDone_=true;
		progress_=1.0;
		cv.notify_all();
	}

	void setException(const exception& e)
	{
		sLock lock(mutex);
		cout << "FUTURE: set exception: " << e << endl;
		exception_(e);
	}

	bool isHalt() const
	{
		sLock lock(mutex);
		return haltOrder_;
	}

public:

	Future():
		isDone_(false),
		progress_(0.0),
		haltOrder_(false),
		exception_(NULL),
		pValue_(NULL)
	{}

	Future(Future<T>& rhs)
	{
		sLock lock(mutex);
		isDone_=rhs.isDone_;
		progress_=rhs.progress_;
		haltOrder_=rhs.haltOrder_;
		exception_(rhs.exception_);
		pValue_(rhs.pValue_);
	}

	Future<T> operator=(Future<T>& rhs)
	{
		return Future<T>(rhs);
	}
	
	T getValue()//powinno blokowac, az bedzie ok; na razie nie mamy wielu w¹tków, wiêc zobaczymy jak to dzia³a za jakieœ 2 miesi¹ce kiedy je w koñcu dodamy
	{
		sLock lock(mutex);
		if(exception_)
		{
			throw exception_;
		}

		/*while(!isDone_)
		{
			cv.wait(lock);
		}*/

		return *pValue_;
	}

	double getProgress()
	{
		sLock lock(mutex);
		return progress_;
	}

	bool isDone()
	{
		sLock(mutex);
		return isDone_;
	}

	bool hasException()
	{
		sLock(mutex);
		return exception_;
	}

	void halt()
	{
		sLock lock(mutex);
		haltOrder_=true;
	}

};

#endif