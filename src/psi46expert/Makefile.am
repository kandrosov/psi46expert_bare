# AM_CPPFLAGS = -I$(srcdir)/.. $(ROOTFLAGS)

# PROGRAMS ----------------------------------------------------------------------------------------------------------------------------------------------------

bin_PROGRAMS = psi46expert

psi46expert_SOURCES = psi46expert.cpp
psi46expert_LDADD = libpsi46expert.la ../BasePixel/libpsi46BasePixel.la ../interface/libpsi46interface.la ../psi/libpsi46common.la \
					$(ROOTLIBS) $(LIBFTD2XX) $(LIBUSB) -lboost_system -lboost_date_time -lboost_thread-mt -lgpib -lreadline
psi46expert_LDFLAGS = -static

# LIBRARIES ---------------------------------------------------------------------------------------------------------------------------------------------------

lib_LTLIBRARIES = libpsi46expert.la

libpsi46expert_la_SOURCES = \
							AddressDecoding.cc \
			                AddressLevels.cc \
			                AnalogReadout.cc \
			                Analysis.cc \
			                BumpBonding.cc \
			                CalDelay.cc \
			                ChipVariation.cc \
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
							BiasVoltageController.cc \
							PsiShell.cc

