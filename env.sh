#!/bin/bash

export ROOTSYS=/home/pixel/tools/root/5.34.03
export LIBFTD2XX_PATH=/home/pixel/tools/libftd2xx/0.4.16
export PSI46_PATH=/home/pixel/tools/psi46expert/107.bare

export LD_LIBRARY_PATH=/usr/lib64:/usr/local/lib:$ROOTSYS/lib:$LIBFTD2XX_PATH/lib
export PATH=/usr/local/bin:/bin:/usr/bin:$ROOTSYS/bin
export LIBRARY_PATH=$LD_LIBRARY_PATH
export CPATH=$LIBFTD2XX_PATH/include:$ROOTSYS/include
