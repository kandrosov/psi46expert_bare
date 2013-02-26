/*!
 * \file hvRead.cpp
 * \brief Measure voltage using Keithley.
 *
 * \b Changelog
 * 25-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Keithley moved into psi namespace.
 * 30-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Changed to support IVoltageSource interface.
 * 24-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Changed to support IHighVoltageSource interface.
 */

#include "BasePixel/Keithley.h"
#include <time.h>
#include <stdio.h>
#include <iostream>

int main(int argc, char* argv[])
{
  psi::Keithley k;
  const psi::IVoltageSource::Measurement m = k.Measure();
  std::cout << m.Voltage << ":" << m.Current << std::endl;
  return 0;
}
