lib_LTLIBRARIES = libpsi46analysis.la
bin_PROGRAMS = psi46report

libpsi46analysis_la_SOURCES = \
							  Analysis.cc

psi46report_SOURCES = psi46report.cpp
psi46report_LDADD = libpsi46analysis.la $(ROOTLIBS) -lboost_program_options
psi46report_LDFLAGS = -static
