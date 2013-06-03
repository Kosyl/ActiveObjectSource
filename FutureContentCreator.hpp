/**
* @file FutureContentCreator.hpp
* @author Michal Kosyl
* @author Marta Kuzak
* Active Object implementation.
* Future allows a client to obtain the state, progress and results of method invocations.
*/
#ifndef _FC_CREATOR_
#define _FC_CREATOR_

#include "Future.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/exception_ptr.hpp>
#include "SimpleLog.hpp"
#include "FutureContent.hpp"

namespace ActiveObject
{
  /**
   * Enables setting progress, state and result of client request. 
   * @brief Writer of FutureContent
   */
  class FutureContentCreator
  {
    
  public:
    /**
     * @brief Constructs FutureContentCreator.
     */
    FutureContentCreator():
    log_("Creator",5)
    {
      DLOG(log_ << "constructor" << endl);
    }
    /**
     * Construct copy of other FutureContentCreator.
     * @brief Copy constructor.
     */ 
    FutureContentCreator(const FutureContentCreator& rhs):
    log_("Creator"),
    pFutureContent_(rhs.pFutureContent_)
    {
      DLOG(log_ << "c constructor" << endl);
    }
    /**
     * @brief Destructor.
     */
    virtual ~FutureContentCreator()
    {
      DLOG(log_ << "destructor" << endl);
    }
    /**
     * @brief Sets FutureContent.
     * @param futureContentPtr Pointer to the FutureContent.
     */
    void setFutureContent(boost::shared_ptr<FutureContent> futureContentPtr)
    {
      DLOG(log_ << "setFutureContent ()" << endl);
      pFutureContent_=futureContentPtr;
    }
    
    /*void resetContentPointer()
     *	{
     *		DLOG(log_ << "setFutureContent ()" << endl);
     *		pFutureContent_.reset();
     }*/
    
    protected:
      /**
       * @brief Sets new progress value.
       * @param progress new progress.
       */
      void setProgress(const double& progress)
      {
		DLOG(log_ << "setProgress (" << progress << ")" << endl);
		pFutureContent_.lock()->setProgress(progress);
      }
      /**
       * @brief Sets state of the method invocation.
       * @param fs new state of method invocation.
       */
      void setState(const FutureState fs)
      {
		DLOG(log_ << "setState (" << fs << ")" << endl);
		pFutureContent_.lock()->setState(fs);
      }
      /**
       * @brief Sets exception of the method invocation.
       * @param e Pointer to the exception.
       */
      void setException(boost::exception_ptr& e)
      {
		DLOG(log_ << "setException ()" << endl);
		pFutureContent_.lock()->setException(e);
      }
      /**
       * @brief Sets value of method request.
       * @param val result of method invocation.
       */
      template<typename T>
      void setValue(const T& val)
      {
		DLOG(log_ << "setValue (" << val << ")" << endl);
		pFutureContent_.lock()->setValue(val);
      }
      /**
       * @brief Indicates whether the invocation is cancelled.
       * @return whether the invocation is cancelled.
       */
      bool isCancelled()
      {
		DLOG(log_ << "isCancelled ()" << endl);
		return pFutureContent_.lock()->isCancelled();
      }
      /**
       * @brief Sets FutureContent state to CANCELLED
       */
      void setCancelled()
      {
		DLOG(log_ << "setException ()" << endl);
		//pFutureContent_->setException(boost::copy_exception(new RequestCancelledException()));
		pFutureContent_.lock()->setState(CANCELLED);
      }
      
      Logger log_;
      
  private:
    /**
     * Pointer to FutureContent. 
     */
    boost::weak_ptr<FutureContent> pFutureContent_;
  };
  
}//ActiveObject
#endif
