#!/bin/bash

export ROOTSYS=/soft/root/5.34.14
#export QT_CREATOR_PATH=/opt/qt/qt-creator/2.6.1-build
#export VALGRIND_PATH=/opt/valgrind/3.8.1
#export LIBFTD2XX_PATH=/opt/libftd2xx/0.4.16
export PSI46_PATH=/soft/psi46expert/107.bare
export PSI46_SRC_PATH=/soft/psi46expert/107.bare.source

#export LD_LIBRARY_PATH=/usr/lib64:/usr/local/lib:$ROOTSYS/lib:$LIBFTD2XX_PATH/lib
export LD_LIBRARY_PATH=/usr/local/lib:/usr/lib:$ROOTSYS/lib
export PATH=/usr/local/bin:/usr/bin:/bin:$ROOTSYS/bin:$PSI46_SRC_PATH/src/psi46expert:$PSI46_SRC_PATH/src/analysis:$PSI46_SRC_PATH/src/scripts
#export PATH=/usr/local/bin:/bin:/usr/bin:$ROOTSYS/bin:$QT_CREATOR_PATH/bin:$VALGRIND_PATH/bin:$PSI46_SRC_PATH/src/psi46expert:$PSI46_SRC_PATH/src/analysis:$PSI46_SRC_PATH/src/scripts
export LIBRARY_PATH=$LD_LIBRARY_PATH
export CPATH=/usr/include:/usr/local/include:$ROOTSYS/include
