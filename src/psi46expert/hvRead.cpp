/*!
 * \file hvRead.cpp
 * \brief Measure voltage using Keithley.
 *
 * \b Changelog
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
  Keithley k;
  const IVoltageSource::Measurement m = k.Measure();
  std::cout << m.Voltage << ":" << m.Current << std::endl;
  return 0;
}
