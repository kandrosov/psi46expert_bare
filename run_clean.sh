#!/bin/bash

make clean

rm -rf aclocal.m4 autom4te.cache config.guess config.log config.status \
       config.sub configure configure.ac depcomp install-sh libtool ltmain.sh \
       m4 Makefile Makefile.in missing

rm -rf src/config.h src/config.h.in src/Makefile src/Makefile.in src/stamp-h1

rm -rf src/BasePixel/.deps src/BasePixel/Makefile src/BasePixel/Makefile.in

rm -rf src/interface/.deps src/interface/Makefile src/interface/Makefile.in

rm -rf src/psi46expert/.deps src/psi46expert/Makefile \
       src/psi46expert/Makefile.in src/psi46expert/#psi46expert.cpp# \
       src/psi46expert/#TestModule.cc#
