/*!
 * \file CalibrationTable.h
 * \brief Definition of CalibrationTable class.
 */

#pragma once

/*!
 * \brief Class provides calibration data for DACs.
 *
 * Vcal calibration data provided and measured by Wolfram Erdmann.
 */
class CalibrationTable {
public:
    static int CorrectedVcalDAC(int value);
    static double VcalDAC(int range, int value);
    static double VcalDAC(int range, double value);
    static int VcalDACInv(int range, double value);
    static void VcalDACOrder(int range);
    static double Slope(int range);
};
