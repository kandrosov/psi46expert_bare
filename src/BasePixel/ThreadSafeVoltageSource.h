/*!
 * \file ThreadSafeVoltageSource.h
 * \brief Definition of ThreadSafeVoltageSource class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include <boost/scoped_ptr.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include "IVoltageSource.h"

/*!
 * \brief Provides thread-safe access to the voltage source.
 */
class ThreadSafeVoltageSource : public IVoltageSource
{
public:
    /*!
     * \brief ThreadSafeVoltageSource constructor.
     * \param aVoltageSource - a pointer to the voltage source.
     *
     * To guarantee thread safety \a aVoltageSource should be accessed only through ThreadSafeVoltageSource object. For
     * that reason \a aVoltageSource will be owned by ThreadSafeVoltageSource object and will be destroyed with it.
     */
    ThreadSafeVoltageSource(IVoltageSource* aVoltageSource);

    /// \copydoc IVoltageSource::Set
    virtual Value Set(const Value& value);

    /// \copydoc IVoltageSource::Measure
    virtual Measurement Measure();

    /// \copydoc IVoltageSource::Off
    virtual void Off();

    /*!
     * \brief Lock the voltage source to be used only in the current thread.
     *
     * The calling thread will be suspended until the lock is obtained. If thread is already own the lock, it will not
     * blocked, however number of Unlock calls should be equal to the number of Lock calls to unlock the object.
     */
    void Lock();

    /*!
     * \brief Unlock the voltage source.
     *
     * If number of Unlock calls will be greater than the number of Lock calls, it will lead to the undefined
     * behaviour.
     */
    void Unlock();

private:
    boost::scoped_ptr<IVoltageSource> voltageSource;
    boost::recursive_mutex mutex;
};
