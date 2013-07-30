/*!
 * \file ThreadSafeVoltageSource.h
 * \brief Definition of ThreadSafeVoltageSource class.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#pragma once

#include <list>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/locks.hpp>

#include "psi/units.h"
#include "IVoltageSource.h"

namespace psi {
/*!
 * \brief Provides thread-safe access to the voltage source.
 */
class ThreadSafeVoltageSource : public IVoltageSource, private boost::noncopyable {
public:
    /// Measurement collection type.
    typedef std::list<IVoltageSource::Measurement> MeasurementCollection;

    /*!
     * \brief ThreadSafeVoltageSource constructor.
     * \param aVoltageSource - a pointer to the voltage source.
     *
     * To guarantee thread safety \a aVoltageSource should be accessed only through ThreadSafeVoltageSource object. For
     * that reason \a aVoltageSource will be owned by ThreadSafeVoltageSource object and will be destroyed with it.
     */
    explicit ThreadSafeVoltageSource(IVoltageSource* aVoltageSource, bool saveMeasurements = true);

    /// \copydoc IVoltageSource::Set
    virtual Value Set(const Value& value);

    /// \copydoc IVoltageSource::Accuracy
    virtual ElectricPotential Accuracy(const ElectricPotential& voltage);

    /// \copydoc IVoltageSource::Measure
    virtual Measurement Measure();

    /// \copydoc IVoltageSource::Off
    virtual void Off();

    /// Gradually change voltage with given voltage step and delay between steps.
    bool GradualSet(const Value& value, const psi::ElectricPotential& step, const psi::Time& delayBetweenSteps,
                    bool checkForCompliance = true);

    /// Returns reference to a collection with all performed measurments.
    /// \remarks ThreadSafeVoltageSource should be locked while accessing the measuremens.
    const MeasurementCollection& Measurements() const { return measurements; }

    /*!
     * \brief Lock the voltage source to be used only in the current thread.
     *
     * The calling thread will be suspended until the lock is obtained. If thread is already own the lock, it will not
     * blocked, however number of Unlock calls should be equal to the number of Lock calls to unlock the object.
     */
    void lock();

    /*!
     * \brief Unlock the voltage source.
     *
     * If number of Unlock calls will be greater than the number of Lock calls, it will lead to the undefined
     * behaviour.
     */
    void unlock();

private:
    boost::recursive_mutex mutex;
    boost::scoped_ptr<IVoltageSource> voltageSource;
    bool saveMeasurements;
    MeasurementCollection measurements;
    Value currentValue;
    bool isOn;
};

}
