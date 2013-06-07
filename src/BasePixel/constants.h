/*!
 * \file constants.h
 * \brief Provides a minimal set of constants for the psi namespace.
 */

#pragma once

namespace psi {
static const unsigned ROCNUMROWS = 80;  // # rows
static const unsigned ROCNUMCOLS = 52;  // # columns
static const unsigned ROCNUMDCOLS = 26;  // # double columns (= columns/2)
static const unsigned MODULENUMROCS = 16; // # max. number of rocs on a module
static const unsigned FIFOSIZE = 4096; // size of the fifo buffer on the analog testboard
}
