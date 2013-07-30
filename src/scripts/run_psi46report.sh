#!/bin/bash
# run_psi46report.sh: run psi46report using an environment organized for the bare module tests.
# Author: Konstantin Androsov <konstantin.androsov@gmail.com>.

echo "run_psi46report.sh: run psi46report using an environment organized for the bare module tests."
echo "Usage: run_psi46report.sh [module_serial_number]"
echo

if [[ $# -lt 1 ]] ; then
	exit
fi

RESULTS_PATH_PREFIX="results"
REPORTS_PATH_PREFIX="reports"

MODULE_SERIAL_NUMBER="$1"
RESULTS_PATH="$RESULTS_PATH_PREFIX/$MODULE_SERIAL_NUMBER"
REPORTS_PATH="$REPORTS_PATH_PREFIX/$MODULE_SERIAL_NUMBER"
#LOG_FILE_NAME="$REPORTS_PATH_PREFIX/${MODULE_SERIAL_NUMBER}.log"

mkdir -p "$REPORTS_PATH"

find $RESULTS_PATH -name "*.root" \
	| sed "s/\(^$RESULTS_PATH_PREFIX\)\(.*\)\/.*\.root\$/--input\n\0\n--output\n$REPORTS_PATH_PREFIX\2\.pdf/" \
	| xargs -d '\n' -n 4 psi46report # --compatibility # > $LOG_FILE_NAME
