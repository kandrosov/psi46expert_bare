AM_CPPFLAGS = -I$(srcdir)/.. $(ROOTFLAGS)

# PROGRAMS ----------------------------------------------------------------------------------------------------------------------------------------------------

bin_PROGRAMS = psi46expert psi46hvOff psi46hvOn psi46hvRead psi46takeData psi46debugData psi46readData

psi46expert_SOURCES = psi46expert.cpp
psi46expert_LDADD = libpsi46expert.la ../BasePixel/libpsi46BasePixel.la ../interface/libpsi46interface.la $(ROOTLIBS) $(LIBFTD2XX) $(LIBUSB) -lboost_system -lboost_date_time -lboost_thread-mt -lgpib -lreadline
psi46expert_LDFLAGS = -static

psi46hvOff_SOURCES = hvOff.cpp
psi46hvOff_LDADD = ../BasePixel/libpsi46BasePixel.la ../interface/libpsi46interface.la $(LIBFTD2XX) $(LIBUSB)
psi46hvOff_LDFLAGS = -static

psi46hvOn_SOURCES = hvOn.cpp
psi46hvOn_LDADD = ../BasePixel/libpsi46BasePixel.la ../interface/libpsi46interface.la $(LIBFTD2XX) $(LIBUSB)
psi46hvOn_LDFLAGS = -static

psi46hvRead_SOURCES = hvRead.cpp
psi46hvRead_LDADD = ../BasePixel/libpsi46BasePixel.la ../interface/libpsi46interface.la $(LIBFTD2XX) $(LIBUSB)
psi46hvRead_LDFLAGS = -static

psi46takeData_SOURCES = takeData.cpp
psi46takeData_LDADD = libpsi46daq.la ../BasePixel/libpsi46BasePixel.la ../interface/libpsi46interface.la $(ROOTLIBS) $(LIBFTD2XX) $(LIBUSB) -lgpib
psi46takeData_LDFLAGS = -static

psi46debugData_SOURCES = debugData.cpp
psi46debugData_LDADD = libpsi46daq.la ../BasePixel/libpsi46BasePixel.la ../interface/libpsi46interface.la $(ROOTLIBS) $(LIBFTD2XX) $(LIBUSB) -lgpib -lboost_thread-mt
psi46debugData_LDFLAGS = -static

psi46readData_SOURCES = readData.cpp
psi46readData_LDADD = libpsi46readdata.la ../BasePixel/libpsi46BasePixel.la ../interface/libpsi46interface.la $(ROOTLIBS) $(LIBFTD2XX) $(LIBUSB)
psi46readData_LDFLAGS = -static

# LIBRARIES ---------------------------------------------------------------------------------------------------------------------------------------------------

lib_LTLIBRARIES = libpsi46expert.la libpsi46daq.la libpsi46ana.la libpsi46readdata.la

libpsi46expert_la_SOURCES = AddressDecoding.cc \
                AddressLevels.cc \
                AnalogReadout.cc \
                Analysis.cc \
                BumpBonding.cc \
                CalDelay.cc \
                ChipVariation.cc \
                CommandLineInterpreter.cc \
                DacDependency.cc \
                DacOverview.cc \
                FigureOfMerit.cc \
                FullTest.cc \
                IVCurve.cc \
                OffsetOptimization.cc \
                PHCalibration.cc \
                PhDacOverview.cc \
                PhDacScan.cc \
                PhNoise.cc \
                PHRange.cc \
                PHTest.cc \
                PixelAlive.cc \
                SCurveTestBeam.cc \
                SCurveTest.cc \
                TBMTest.cc \
                TBMUbCheck.cc \
                TemperatureCalibration.cc \
                TemperatureTest.cc \
                Test.cc \
                TestControlNetwork.cc \
                TestDoubleColumn.cc \
                TestModule.cc \
                TestPixel.cc \
                TestRange.cc \
                TestRoc.cc \
                ThrComp.cc \
                ThresholdMap.cc \
                ThresholdTest.cc \
                TimeWalkStudy.cc \
                TrimBits.cc \
                Trim.cc \
                TrimLow.cc \
                TrimVcal.cc \
                UbCheck.cc \
                VhldDelOptimization.cc \
                VsfOptimization.cc \
                VsfScan.cc \
                Xray.cc \
                BareTest.cc \
                DataStorage.cc \
				BiasVoltageController.cc

libpsi46daq_la_SOURCES = daqLoggingManager.cc \
             UsbDaq.cc \
             histogrammer.cc

libpsi46daq_la_LIBADD = libpsi46expert.la

libpsi46ana_la_SOURCES = SCurve.cc \
             ../BasePixel/CalibrationTable.cc \
             PHCalibrationFit.cc

libpsi46readdata_la_SOURCES = UsbDaq.cc \
                  histogrammer.cc

# MENTION ALL HEADERS SO THEY GET DISTRIBUTED -----------------------------------------------------------------------------------------------------------------

noinst_HEADERS = AddressDecoding.h \
         AddressLevels.h \
         AnalogReadout.h \
         Analysis.h \
         anaQA.h \
         BumpBonding.h \
         CalDelay.h \
         ChipVariation.h \
         CommandLineInterpreter.h \
         DacDependency.h \
         DacOverview.h \
         daqDACSettings.hh \
         daqLoggingManager.hh \
         daqMultiDACSettings.hh \
         daqTBSettings.hh \
         FigureOfMerit.h \
         FullTest.h \
         histogrammer.h \
         IVCurve.h \
         OffsetOptimization.h \
         PHCalibrationFit.h \
         PHCalibration.h \
         PhDacOverview.h \
         PhDacScan.h \
         PhNoise.h \
         PHRange.h \
         PHTest.h \
         PixelAlive.h \
         SCurve.h \
         SCurveTestBeam.h \
         SCurveTest.h \
         TBMTest.h \
         TBMUbCheck.h \
         TemperatureCalibration.h \
         TemperatureTest.h \
         TestControlNetwork.h \
         TestDoubleColumn.h \
         Test.h \
         TestModule.h \
         TestParameters.h \
         TestPixel.h \
         TestRange.h \
         TestRoc.h \
         ThrComp.h \
         ThresholdMap.h \
         ThresholdTest.h \
         TimeWalkStudy.h \
         TrimBits.h \
         Trim.h \
         TrimLow.h \
         TrimVcal.h \
         UbCheck.h \
         UsbDaq.h \
         VhldDelOptimization.h \
         VsfOptimization.h \
         VsfScan.h \
         Xray.h

