/*!
 * \file CalibrationTable.h
 * \brief Definition of CalibrationTable class.
 *
 * \b Changelog
 * 09-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Corrected questionable language constructions, which was found using -Wall g++ option.
 *      - Removed unused methods SetVcalVthr, VthrDAC, VthrDACInv.
 */

#pragma once

/*!
 * \brief Class provides calibration data for DACs.
 *
 * Vcal calibration data provided and measured by Wolfram Erdmann.
 */
class CalibrationTable
{
public:
    static int CorrectedVcalDAC(int value);
    static double VcalDAC(int range, int value);
    static double VcalDAC(int range, double value);
    static int VcalDACInv(int range, double value);
    static void VcalDACOrder(int range);
    static double Slope(int range);
};
