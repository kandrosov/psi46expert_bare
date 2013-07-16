# PROGRAMS ----------------------------------------------------------------------------------------------------------------------------------------------------

bin_PROGRAMS = psi46expert

psi46expert_SOURCES = psi46expert.cpp
psi46expert_LDADD = libpsi46expert.la ../BasePixel/libpsi46BasePixel.la ../interface/libpsi46interface.la ../psi/libpsi46common.la \
					../tests/libpsi46tests.la $(ROOTLIBS) $(LIBFTD2XX) $(LIBUSB) -lboost_system -lboost_date_time -lboost_thread-mt -lboost_program_options \
					-lgpib -lreadline
psi46expert_LDFLAGS = -static

# LIBRARIES ---------------------------------------------------------------------------------------------------------------------------------------------------

lib_LTLIBRARIES = libpsi46expert.la

libpsi46expert_la_SOURCES = \
			                TestControlNetwork.cc \
			                TestDoubleColumn.cc \
			                TestModule.cc \
			                TestPixel.cc \
			                TestRoc.cc \
							BiasVoltageController.cc \
							PsiShell.cc \
							TestBoardFactory.cc

