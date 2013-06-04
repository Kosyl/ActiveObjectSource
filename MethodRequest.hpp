/**
* @file MethodRequest.hpp
* @author Michal Kosyl
* @author Marta Kuzak
* @details Active Object implementation.
* @details MethodRequest passes information about client request such as method parametres or type of result from Proxy to Scheduler.
*/
#ifndef _COMMAND_
#define _COMMAND_

#include "Future.hpp"
#include "SimpleLog.hpp"
#include <boost/function.hpp>

namespace ActiveObject
{

	using namespace std;
	/**
	* @brief NullCommandException thrown when try to refer to method that is not ready. {tak wynika z kodu, KARDAMON!)
	*/
	class NullCommandException: public exception{};

	/**
	* It passes context information about a specific method invocation on a Proxy, such as method parameters and code,
	* from the Proxy to a Scheduler running in a separate thread. 
	* @brief An abstract class which defines an interface for executing methods.
	* @tparam Servant Type of Servant that executes method.
	*/
	template<class Servant>
	class Functor
	{

	protected:

		/**
		* Pointer to FutureContent which contains result, progress and state of method invocation.
		*/
		boost::shared_ptr<FutureContent> content_;
		/**
		* Thread-safe logger
		*/
		DLOG(Logger log_;)

		/**
		* Guard function to be called to check the given synchronization constraints
		*/
		unique_ptr<boost::function<bool(Servant*)> > guard_;

	public:

		/**
		* @brief Constructs Functor with a given FutureContent
		* @param content Pointer to FutureContent
		*/
		Functor(boost::shared_ptr<FutureContent> content):
			content_(content)
		{
			DLOG(log_.setName("Functor"));
			DLOG(log_.setColor(7));
		}

		/**
		* @brief Constructs Functor with a given FutureContent and guard function.
		* @param content Pointer to FutureContent
		* @param guard guard function
		*/
		Functor(boost::shared_ptr<FutureContent> content, boost::function<bool(Servant*)> guard):
			content_(content),
			guard_(new boost::function<bool(Servant*)>)
		{
			DLOG(log_.setName("Functor"));
			DLOG(log_.setColor(7));
			*guard_=guard;
		}

		/**
		* Destructor
		*/
		virtual ~Functor()
		{
		}

		/**
		* @brief Makes given servant execute method.
		* @param servant Pointer to servant
		*/
		virtual void execute(boost::shared_ptr<Servant> servant)=0;

		/**
		* @brief Returns pointer to FutureContent.
		* @return pointer to FutureContent that contains info about the method invocation.
		*/
		boost::shared_ptr<FutureContent> getFutureContent()
		{
			DLOG(log_ << "getFutureContent()" << endl);
			return content_;
		}

		/**
		* @brief Checks some custom synchronization constraints
		* @return false if all of the custom constraints requiret to execute the method are met; true otherwise (the method is "guarded").
		*/
		bool guard(boost::shared_ptr<Servant> s)
		{
			DLOG(log_ << "guard" << endl);
			if(guard_.get()==NULL) return false;

			return (*guard_)(s.get());
		}

		/**
		* @brief Checks if the command contained within is not null.
		* @return true if the command is not null; false otherwise.
		*/
		virtual bool isReady()=0;
	};//Functor

	/**
	* It passes context information about a specific method invocation on a Proxy, such as method parameters and code,
	* from the Proxy to a Scheduler running in a separate thread. 
	* @brief Implements Functor interface.
	* @tparam ReturnType Type of return value of the invoked method.
	* @tparam Servant Type(class) of servant that executes method.
	*/
	template<typename ReturnType, class Servant>
	class MethodRequest:public Functor<Servant>
	{

	private:

		/**
		* command object to be executed on a given Servant
		*/
		boost::function<ReturnType(Servant*)> command_;

	public:

		/**
		* @brief Constructs MethodRequest with given command and FutureContent.
		* @param f Invoked command
		* @param content Pointer to FutureContent that contains info about the invoked command.
		*/
		MethodRequest(boost::function<ReturnType(Servant*)> f, boost::shared_ptr<FutureContent> content):
			Functor<Servant>(content),
			command_(f)
		{
			DLOG(this->log_ << "constructor" << endl);
		}

		/**
		* @brief Constructs MethodRequest with given command and FutureContent.
		* @param f Invoked command
		* @param content Pointer to FutureContent that contains info about the invoked command.
		* @param guard guard function
		*/
		MethodRequest(boost::function<ReturnType(Servant*)> f, boost::shared_ptr<FutureContent> content, boost::function<bool(Servant*)> guard):
			Functor<Servant>(content, guard),
			command_(f)
		{
			DLOG(this->log_ << "constructor" << endl);
		}

		/**
		* @brief Implementation of Functor::execute
		* @param servant Servant that executes the method.
		* @see virtual void Functor::execute(boost::shared_ptr<Servant> servant)
		* @throw NullCommandException when the method is not ready.
		*/
		virtual void execute(boost::shared_ptr<Servant> servant)
		{
			DLOG(this->log_ << "execute() - begin" << endl);
			if(isReady())
			{
				try
				{
					DLOG(this->log_ << "execute() - isReady==true, executing..." << endl);
					this->content_->setValue(command_(servant.get()));
				}
				catch(RequestCancelledException)
				{
					DLOG(this->log_ << "execute() - request cancelled" << endl);
				}
				catch(...)
				{
					DLOG(this->log_ << "execute() - exception" << endl);
					this->content_->setException(boost::current_exception());
				}
				DLOG(this->log_ << "execute() - finished" << endl);
			}
			else
			{
				DLOG(this->log_ << "execute() - isReady==false" << endl);
				throw NullCommandException();
			}
		}

		/**
		* @brief Says if the MethodRequest is ready to be executed.
		* @return whether MethodRequest is ready.
		*/
		virtual bool isReady()
		{
			return (((bool)command_)!=false && this->content_!=NULL);
		}

		/**
		* Destructor.
		*/
		virtual ~MethodRequest()
		{
			DLOG(this->log_ << "destructor" << endl);
		}

	};//MethodRequest

	/**
	* Void specialization of MethodRequest. Differs in the way the result value is set.
	* It passes context information about a specific method invocation on a Proxy, such as method parameters and code,
	* from the Proxy to a Scheduler running in a separate thread. 
	* @brief Implements Functor interface.
	* @tparam Servant Type(class) of servant that executes method.
	*/
	template<class Servant>
	class MethodRequest<void,Servant>:public Functor<Servant>
	{

	private:
		/**
		* Pointer to command
		*/
		boost::function<void(Servant*)> command_;

	public:

		/**
		* @brief Constructs MethodRequest with given command and FutureContent.
		* @param f Invoked command
		* @param content Pointer to FutureContent that contains info about the invoked command.
		*/
		MethodRequest(boost::function<void(Servant*)> f, boost::shared_ptr<FutureContent> content):
			Functor<Servant>(content),
			command_(f)
		{
			DLOG(this->log_ << "constructor" << endl);
		}

		/**
		* @brief Constructs MethodRequest with given command and FutureContent.
		* @param f Invoked command
		* @param content Pointer to FutureContent that contains info about the invoked command.
		* @param guard guard function
		*/
		MethodRequest(boost::function<void(Servant*)> f, boost::shared_ptr<FutureContent> content, boost::function<bool(Servant*)> guard):
			Functor<Servant>(content, guard),
			command_(f)
		{
			DLOG(this->log_ << "constructor" << endl);
		}

		/**
		* @brief Implementation of Functor::execute
		* @param servant Servant that is to execute the method.
		* @see virtual void Functor::execute(boost::shared_ptr<Servant> servant)
		* @throw NullCommandException when the method is not ready.
		*/
		virtual void execute(boost::shared_ptr<Servant> servant)
		{
			DLOG(this->log_ << "execute() - begin" << endl);
			if(isReady())
			{
				try
				{
					DLOG(this->log_ << "execute() - isReady==true, executing..." << endl);
					command_(servant.get());
					this->content_->setValue(true);
				}
				catch(RequestCancelledException)
				{
					DLOG(this->log_ << "execute() - request cancelled" << endl);
				}
				catch(...)
				{
					DLOG(this->log_ << "execute() - exception" << endl);
					this->content_->setException(boost::current_exception());
				}
				DLOG(this->log_ << "execute() - finished" << endl);
			}
			else
			{
				DLOG(this->log_ << "execute() - isReady==false" << endl);
				throw NullCommandException();
			}
		}

		/**
		* @brief Says if the MethodRequest is ready.
		* @return whether MethodRequest is ready.
		*/
		virtual bool isReady()
		{
			return (((bool)command_)!=false && this->content_!=NULL);
		}

		/**
		* Destructor.
		*/
		virtual ~MethodRequest()
		{
			DLOG(this->log_ << "destructor" << endl);
		}

	};//MethodRequest<void>

}//ActiveObject
#endif
