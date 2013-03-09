/*!
 * \file CalibrationTable.cc
 * \brief Implementation of CalibrationTable class.
 *
 * \b Changelog
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using a new PSI Logging System.
 */
#include "BasePixel/CalibrationTable.h"
#include "psi/log.h"

// == Vcal Calibration ===========================================================


// -- returns DAC registers sorted by increasing cal-voltage, i.e. VcalDAC(x, CorrectedVcalDAC(i)) <= VcalDAC(x, CorrectedVcalDAC(i + 1)). The order is the same for both ranges
int CalibrationTable::CorrectedVcalDAC(int value)
{
	static const int vcal[256] = {0, 1, 2, 3, 4, 5, 6, 8, 7, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 24, 23, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 40, 39, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 56, 55, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 72, 71, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 88, 87, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 104, 103, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 120, 119, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 136, 135, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 152, 151, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 184, 183, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 200, 199, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 216, 215, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 232, 231, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 248, 247, 249, 250, 251, 252, 253, 254, 255};
	return vcal[value];
}

// -- returns the measured voltage for the DAC value
double CalibrationTable::VcalDAC(int range, int value)
{
	static const double vcal0[256]={0.0000,0.0015,0.0028,0.0042,0.0053,0.0053,0.0080,0.0094,0.0089,0.0103,0.0116,0.0130,0.0141,0.0156,0.0169,0.0183,0.0205,0.0219,0.0232,0.0246,0.0256,0.0269,0.0283,0.0296,0.0292,0.0305,0.0319,0.0332,0.0343,0.0357,0.0371,0.0384,0.0404,0.0417,0.0430,0.0444,0.0454,0.0467,0.0481,0.0494,0.0489,0.0503,0.0516,0.0530,0.0541,0.0555,0.0568,0.0582,0.0604,0.0617,0.0630,0.0643,0.0653,0.0667,0.0680,0.0693,0.0688,0.0702,0.0715,0.0728,0.0739,0.0753,0.0766,0.0780,0.0805,0.0818,0.0831,0.0844,0.0854,0.0867,0.0880,0.0894,0.0889,0.0902,0.0915,0.0929,0.0940,0.0953,0.0967,0.0967,0.1002,0.1015,0.1028,0.1041,0.1051,0.1064,0.1077,0.1090,0.1085,0.1099,0.1112,0.1125,0.1136,0.1149,0.1163,0.1176,0.1196,0.1209,0.1221,0.1235,0.1244,0.1257,0.1270,0.1283,0.1278,0.1291,0.1304,0.1318,0.1329,0.1342,0.1355,0.1369,0.1391,0.1404,0.1417,0.1430,0.1439,0.1452,0.1465,0.1478,0.1472,0.1486,0.1499,0.1512,0.1523,0.1536,0.1550,0.1563,0.1569,0.1582,0.1594,0.1607,0.1616,0.1629,0.1642,0.1655,0.1649,0.1662,0.1675,0.1689,0.1700,0.1713,0.1726,0.1740,0.1762,0.1775,0.1787,0.1800,0.1809,0.1822,0.1834,0.1848,0.1841,0.1854,0.1867,0.1881,0.1891,0.1905,0.1918,0.1933,0.1952,0.1965,0.1977,0.1991,0.2001,0.2011,0.2021,0.2031,0.2031,0.2041,0.2051,0.2071,0.2081,0.2091,0.2111,0.2121,0.2141,0.2161,0.2171,0.2181,0.2191,0.2201,0.2211,0.2231,0.2221,0.2231,0.2241,0.2261,0.2271,0.2281,0.2301,0.2311,0.2341,0.2341,0.2361,0.2371,0.2381,0.2391,0.2411,0.2421,0.2411,0.2421,0.2441,0.2451,0.2461,0.2471,0.2491,0.2491,0.2531,0.2541,0.2551,0.2561,0.2571,0.2581,0.2591,0.2611,0.2601,0.2611,0.2621,0.2641,0.2651,0.2661,0.2681,0.2691,0.2721,0.2731,0.2741,0.2751,0.2761,0.2771,0.2781,0.2791,0.2781,0.2801,0.2811,0.2821,0.2831,0.2851,0.2861,0.2881,0.2901,0.2921,0.2931,0.2941,0.2941,0.2951,0.2961,0.2981,0.2971,0.2981,0.2991,0.3011,0.3021,0.3031,0.3051,0.3071};
	static const double vcal1[256]={0.0000,0.0093,0.0183,0.0274,0.0344,0.0436,0.0525,0.0616,0.0586,0.0678,0.0767,0.0859,0.0933,0.1025,0.1114,0.1207,0.1355,0.1444,0.1531,0.1620,0.1689,0.1689,0.1865,0.1955,0.1924,0.2017,0.2097,0.2197,0.2267,0.2357,0.2447,0.2537,0.2667,0.2757,0.2837,0.2927,0.2997,0.3087,0.3167,0.3257,0.3227,0.3317,0.3407,0.3497,0.3567,0.3657,0.3747,0.3837,0.3977,0.4067,0.4157,0.4237,0.4307,0.4397,0.4487,0.4567,0.4537,0.4627,0.4717,0.4807,0.4877,0.4967,0.5047,0.5137,0.5307,0.5397,0.5477,0.5567,0.5627,0.5717,0.5797,0.5887,0.5857,0.5947,0.6027,0.6117,0.6187,0.6277,0.6367,0.6457,0.6597,0.6687,0.6767,0.6857,0.6917,0.7007,0.7087,0.7177,0.7147,0.7227,0.7317,0.7397,0.7477,0.7557,0.7647,0.7737,0.7867,0.7947,0.8027,0.8117,0.8177,0.8267,0.8347,0.8427,0.8397,0.8487,0.8567,0.8657,0.8727,0.8817,0.8897,0.8897,0.9137,0.9217,0.9297,0.9377,0.9447,0.9527,0.9607,0.9697,0.9657,0.9747,0.9827,0.9917,0.9987,1.0067,1.0157,1.0247,1.0277,1.0367,1.0447,1.0527,1.0587,1.0667,1.0747,1.0837,1.0797,1.0877,1.0967,1.1047,1.1117,1.1207,1.1297,1.1377,1.1527,1.1607,1.1687,1.1767,1.1817,1.1907,1.1977,1.2067,1.2027,1.2107,1.2187,1.2277,1.2347,1.2427,1.2517,1.2607,1.2727,1.2807,1.2887,1.2967,1.3017,1.3097,1.3177,1.3257,1.3207,1.3297,1.3377,1.3457,1.3527,1.3607,1.3697,1.3787,1.3917,1.3997,1.4067,1.4147,1.4187,1.4267,1.4337,1.4417,1.4367,1.4447,1.4527,1.4597,1.4657,1.4737,1.4817,1.4897,1.5027,1.5097,1.5147,1.5207,1.5237,1.5297,1.5347,1.5407,1.5367,1.5427,1.5477,1.5537,1.5567,1.5627,1.5677,1.5727,1.5797,1.5837,1.5867,1.5897,1.5917,1.5947,1.5977,1.6017,1.5987,1.6027,1.6057,1.6087,1.6107,1.6147,1.6177,1.6177,1.6247,1.6267,1.6287,1.6307,1.6317,1.6337,1.6357,1.6377,1.6367,1.6387,1.6407,1.6427,1.6447,1.6467,1.6487,1.6517,1.6547,1.6567,1.6577,1.6587,1.6597,1.6607,1.6627,1.6637,1.6627,1.6647,1.6657,1.6677,1.6687,1.6707,1.6717,1.6737};

	if (range == 1)
	{
		return vcal1[CorrectedVcalDAC(value)];
	}
	else
	{
		return vcal0[CorrectedVcalDAC(value)];
	}
}


double CalibrationTable::Slope(int range)
{
	return VcalDAC(range, 255)/255.;
}

// -- returns the interpolated voltage for the DAC value
double CalibrationTable::VcalDAC(int range, double value)
{
	double lower = VcalDAC(range, CorrectedVcalDAC((int)value));
	double upper = VcalDAC(range, CorrectedVcalDAC((int)value + 1));
	return lower + (upper-lower)*(value - floor(value));
}


// -- returns the vcal DAC to which the measured voltage value corresponds to
int CalibrationTable::VcalDACInv(int range, double value)
{
	for (int i = 0; i < 256; i++)
	{
		if (VcalDAC(range, i) > value) {return i;}
	}
	return 255;
}


// -- helper function to determine the corrected DAC order
void CalibrationTable::VcalDACOrder(int range)
{
	double lastMinimum = -1., minimum = 9999., x;
	int minimumDAC;
	for (int i = 0; i < 256; i++) {

		for (int k = 0; k < 256; k++) {
			x = VcalDAC(range,k);
			if ((x < minimum) && (x > lastMinimum))
			{
				minimumDAC = k;
				minimum = x;
			}
		}
		
		for (int k = 0; k < 256; k++) {	
			x = VcalDAC(range,k);
			if (x == minimum)
			{
                psi::LogInfo() << k << ", ";
			}
		}
		
		lastMinimum = minimum;
		minimum = 9999.;
	}
}

// == Vthr Calibration ===========================================================


// -- sets the vcal value which corresponds to a vthr value (e.g. measured by a Vthr-Vcal curve);
void CalibrationTable::SetVcalVthr(int vthr, int vcal, int range)
{
	vcalVthrTable[range][vthr] = vcal;
}


// -- gives back the measured vcal voltage which corresponds to a vthr value
double CalibrationTable::VthrDAC(int range, int vthr)
{
	return VcalDAC(range, vcalVthrTable[range][vthr]);
}

// -- returns the vthr DAC to which the measured voltage value corresponds to
int CalibrationTable::VthrDACInv(int range, double value)
{
	for (int i = 0; i < 256; i++)
	{
		if (VthrDAC(range, i) < value) {return i;}
	}
	return 255;
}
