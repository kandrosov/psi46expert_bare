/*!
 * \file ThreadSafeVoltageSource.cc
 * \brief Implementation of ThreadSafeVoltageSource class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#include "ThreadSafeVoltageSource.h"
#include "psi_exception.h"

ThreadSafeVoltageSource::ThreadSafeVoltageSource(IVoltageSource* aVoltageSource)
    : voltageSource(aVoltageSource)
{
    if(!aVoltageSource)
        THROW_PSI_EXCEPTION("[ThreadSafeVoltageSource] Voltage source can't be null.")
}

IVoltageSource::Value ThreadSafeVoltageSource::Set(const Value& value)
{
    try
    {
        Lock();
        const IVoltageSource::Value result = voltageSource->Set(value);
        Unlock();
        return result;
    }
    catch(psi_exception&)
    {
        Unlock();
        throw;
    }
}

IVoltageSource::Measurement ThreadSafeVoltageSource::Measure()
{
    try
    {
        Lock();
        const IVoltageSource::Measurement result = voltageSource->Measure();
        Unlock();
        return result;
    }
    catch(psi_exception&)
    {
        Unlock();
        throw;
    }
}

void ThreadSafeVoltageSource::Off()
{
    try
    {
        Lock();
        voltageSource->Off();
        Unlock();
    }
    catch(psi_exception&)
    {
        Unlock();
        throw;
    }
}

void ThreadSafeVoltageSource::Lock()
{
    mutex.lock();
}

void ThreadSafeVoltageSource::Unlock()
{
    mutex.unlock();
}
